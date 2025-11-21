/*
 * ExampleDetectorDecoder.cpp
 *
 *  Created on: 23 Feb 2017
 *      Author: gnx91527
 */

#include "ExampleDetectorDecoder.h"


namespace FrameReceiver
{

ExampleDetectorDecoder::ExampleDetectorDecoder() :
                FrameDecoderUDP(),
                current_frame_(1),
                packet_counter_(0),
        		current_frame_seen_(-1),
        		current_frame_buffer_id_(-1),
        		current_frame_buffer_(0),
        		current_frame_header_(0),
        		dropping_frame_data_(false),
        		frame_timeout_ms_(1000),
        		frames_timedout_(0)
{
  current_raw_packet_header_.reset(new uint8_t[ExampleDetector::packet_header_size]);
 //   dropped_frame_buffer_.reset(new uint8_t[LATRD::total_frame_size]);

    this->logger_ = Logger::getLogger("FR.ExampleDetectorDecoderPlugin");
    LOG4CXX_INFO(logger_, "ExampleDetectorDecoder version " << this->get_version_long() << " loaded");
}

void ExampleDetectorDecoder::init(LoggerPtr& logger, OdinData::IpcMessage& config_msg)
{
	FrameDecoder::init(logger, config_msg);
}

ExampleDetectorDecoder::~ExampleDetectorDecoder()
{
}

const size_t ExampleDetectorDecoder::get_frame_buffer_size() const
{
    return ExampleDetector::total_frame_size;
}

const size_t ExampleDetectorDecoder::get_frame_header_size() const
{
    return sizeof(ExampleDetector::FrameHeader);
}

const size_t ExampleDetectorDecoder::get_packet_header_size() const
{
    return ExampleDetector::packet_header_size;
}

void* ExampleDetectorDecoder::get_packet_header_buffer()
{
    return current_raw_packet_header_.get();
}

void ExampleDetectorDecoder::log_packet(size_t bytes_received, int port, struct sockaddr_in* from_addr)
{
}

void ExampleDetectorDecoder::process_packet_header(size_t bytes_received, int port, struct sockaddr_in* from_addr)
{
  // Check the frame number.  If it is a new frame attempt to pop one off the empty queue.
  current_packet_header_ = reinterpret_cast<ExampleDetector::PacketHeader*>(get_packet_header_buffer());

  if (current_packet_header_->frame_number != current_frame_seen_){
    current_frame_seen_ = current_packet_header_->frame_number;

    if (frame_buffer_map_.count(current_frame_seen_) == 0){
      if (empty_buffer_queue_.empty()){
        current_frame_buffer_ = dropped_frame_buffer_.get();
        if (!dropping_frame_data_){
          LOG4CXX_ERROR(logger_, "First packet from frame " << current_frame_seen_ << " detected but no free buffers available. Dropping packet data for this frame");
          dropping_frame_data_ = true;
        }
      } else {
        current_frame_buffer_id_ = empty_buffer_queue_.front();
        empty_buffer_queue_.pop();
        frame_buffer_map_[current_frame_seen_] = current_frame_buffer_id_;
        current_frame_buffer_ = buffer_manager_->get_buffer_address(current_frame_buffer_id_);

        // Initialise frame header
        current_frame_header_ = reinterpret_cast<ExampleDetector::FrameHeader*>(current_frame_buffer_);
        current_frame_header_->frame_number = current_frame_seen_;
        current_frame_header_->frame_state = FrameDecoder::FrameReceiveStateIncomplete;
        current_frame_header_->packets_received = 0;
        memset(current_frame_header_->packet_state, 0, ExampleDetector::num_packets);
        gettime(reinterpret_cast<struct timespec*>(&(current_frame_header_->frame_start_time)));

        if (!dropping_frame_data_){
          LOG4CXX_DEBUG_LEVEL(2, logger_, "First packet from frame " << current_frame_seen_ << " detected, allocating frame buffer ID " << current_frame_buffer_id_);
        } else {
          dropping_frame_data_ = false;
          LOG4CXX_DEBUG_LEVEL(2, logger_, "Free buffer now available for frame " << current_frame_seen_ << ", allocating frame buffer ID " << current_frame_buffer_id_);
        }
      }
    }
  } else {
    current_frame_buffer_id_ = frame_buffer_map_[current_frame_seen_];
    current_frame_buffer_ = buffer_manager_->get_buffer_address(current_frame_buffer_id_);
    current_frame_header_ = reinterpret_cast<ExampleDetector::FrameHeader*>(current_frame_buffer_);   
  }
  // Update packet_number state map in frame header
  LOG4CXX_DEBUG_LEVEL(1, logger_, "  Setting frame " << current_frame_seen_<< " buffer ID: " << current_frame_buffer_id_ << " packet header index: " << current_frame_header_->packets_received);
  current_frame_header_->packet_state[current_frame_header_->packets_received] = 1;
}

void ExampleDetectorDecoder::reset_statistics(void)
{
LOG4CXX_ERROR(logger_, "Reset statistics");
  packet_counter_ = 0;
}

void* ExampleDetectorDecoder::get_next_payload_buffer() const
{
  uint8_t* next_receive_location = reinterpret_cast<uint8_t*>(current_frame_buffer_)
			+ get_frame_header_size()
			+ ((ExampleDetector::packet_size - ExampleDetector::packet_header_size) * current_packet_header_->packet_number);

  return reinterpret_cast<void*>(next_receive_location);
}

size_t ExampleDetectorDecoder::get_next_payload_size() const
{
    return current_packet_header_->payload_size;
}

FrameDecoder::FrameReceiveState ExampleDetectorDecoder::process_packet(size_t bytes_received, int port, struct sockaddr_in* from_addr)
{
	// Set the frame state to incomplete for this frame
  FrameDecoder::FrameReceiveState frame_state = FrameDecoder::FrameReceiveStateIncomplete;

  // Increment the number of packets received for this frame
  if (!dropping_frame_data_) {
    current_frame_header_->packets_received++;
    LOG4CXX_DEBUG_LEVEL(2, logger_, "  Packet count: " << current_frame_header_->packets_received << " for frame: " << current_frame_header_->frame_number);
  }

	// Check to see if the number of packets we have received is equal to the total number of packets for this frame
	if (current_frame_header_->packets_received == ExampleDetector::num_packets){
    // We have received the correct number of packets, the frame is complete
    // Set frame state accordingly
    frame_state = FrameDecoder::FrameReceiveStateComplete;

		// Complete frame header
    current_frame_header_->frame_state = frame_state;

		// Check we are not dropping data for this frame
		if (!dropping_frame_data_){
			// Erase frame from buffer map
			frame_buffer_map_.erase(current_frame_seen_);

			// Notify main thread that frame is ready
			ready_callback_(current_frame_buffer_id_, current_frame_seen_);

			// Reset current frame seen ID so that if next frame has same number (e.g. repeated
			// sends of single frame 0), it is detected properly
      current_frame_seen_ = -1;
		}
	}
	return frame_state;
}

//! Get the current status of the frame decoder.
//!
//! This method populates the IpcMessage passed by reference as an argument with decoder-specific
//! status information, e.g. packet loss by source.
//!
//! \param[in] param_prefix - path to be prefixed to each status parameter name
//! \param[in] status_msg - reference to IpcMesssage to be populated with parameters
//!
void ExampleDetectorDecoder::get_status(const std::string param_prefix,
                                   OdinData::IpcMessage& status_msg)
{
  status_msg.set_param(param_prefix + "name", std::string("ExampleDetectorFrameDecoder"));
  status_msg.set_param(param_prefix + "packets", packet_counter_);
}

void ExampleDetectorDecoder::monitor_buffers()
{
  int frames_timedout = 0;
  struct timespec current_time;

  gettime(&current_time);

  // Loop over frame buffers currently in map and check their state
  std::map<int, int>::iterator buffer_map_iter = frame_buffer_map_.begin();
  while (buffer_map_iter != frame_buffer_map_.end())
  {
    int frame_num = buffer_map_iter->first;
    int buffer_id = buffer_map_iter->second;
    void* buffer_addr = buffer_manager_->get_buffer_address(buffer_id);
    ExampleDetector::FrameHeader* frame_header = reinterpret_cast<ExampleDetector::FrameHeader*>(buffer_addr);

    if (elapsed_ms(frame_header->frame_start_time, current_time) > frame_timeout_ms_){
      LOG4CXX_DEBUG_LEVEL(1, logger_, "Frame " << frame_num << " in buffer " << buffer_id
        << " addr 0x" << std::hex << buffer_addr << std::dec
        << " timed out with " << frame_header->packets_received << " packets received");

      if (current_frame_seen_ == frame_num){
        current_frame_seen_ = -1;
      }
      frame_header->frame_state = FrameReceiveStateTimedout;
      ready_callback_(buffer_id, frame_num);
      frames_timedout++;
      frame_buffer_map_.erase(buffer_map_iter++);
    } else {
      buffer_map_iter++;
    }
  }
  if (frames_timedout){
    LOG4CXX_WARN(logger_, "Released " << frames_timedout << " timed out incomplete frames");
  }
  frames_timedout_ += frames_timedout;

  LOG4CXX_DEBUG_LEVEL(2, logger_, get_num_mapped_buffers() << " frame buffers in use, "
    << get_num_empty_buffers() << " empty buffers available, "
    << frames_timedout_ << " incomplete frames timed out");
}

unsigned int ExampleDetectorDecoder::elapsed_ms(struct timespec& start, struct timespec& end)
{
    double start_ns = ((double)start.tv_sec * 1000000000) + start.tv_nsec;
    double end_ns   = ((double)  end.tv_sec * 1000000000) +   end.tv_nsec;

    return (unsigned int)((end_ns - start_ns)/1000000);
}

/**
  * Get the plugin major version number.
  *
  * \return major version number as an integer
  */
int ExampleDetectorDecoder::get_version_major()
{
    return 0;
}

/**
  * Get the plugin minor version number.
  *
  * \return minor version number as an integer
  */
int ExampleDetectorDecoder::get_version_minor()
{
    return 0;
}

/**
  * Get the plugin patch version number.
  *
  * \return patch version number as an integer
  */
int ExampleDetectorDecoder::get_version_patch()
{
    return 0;
}

/**
  * Get the plugin short version (e.g. x.y.z) string.
  *
  * \return short version as a string
  */
std::string ExampleDetectorDecoder::get_version_short()
{
    return "0.0.0";
}

/**
  * Get the plugin long version (e.g. x.y.z-qualifier) string.
  *
  * \return long version as a string
  */
std::string ExampleDetectorDecoder::get_version_long()
{
    return "0.0.0";
}

} /* namespace FrameReceiver */
