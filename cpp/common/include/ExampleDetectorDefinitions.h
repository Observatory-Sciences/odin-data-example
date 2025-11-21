/*
 * ExampleDetectorDefinitions.h
 *
 *  Created on: 17 November 2025
 *      Author: Alan Greer
 */

#ifndef INCLUDE_EXAMPLEDETECTORDEFINITIONS_H_
#define INCLUDE_EXAMPLEDETECTORDEFINITIONS_H_

#include "gettime.h"
#include <stdint.h>
#include <time.h>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <time.h>

namespace ExampleDetector
{
  static const size_t packet_size        = 268; // 3x4 bit header plus 256 bytes payload
  static const size_t num_packets        = 256; // Number of primary packets in a buffer
  static const size_t packet_header_size = 12;   // 3x32 bit ints packet header

  static const size_t frame_width        = 256;
  static const size_t frame_height       = 256;

  typedef struct
  {
  	uint32_t frame_number;
  	uint32_t packet_number;
    uint32_t payload_size;
  } PacketHeader;

  typedef struct
  {
    uint32_t frame_number;
    uint32_t frame_state;
    struct timespec frame_start_time;
    uint32_t packets_received;
    uint8_t  packet_state[num_packets];
  } FrameHeader;

  static const size_t data_size           = (packet_size - packet_header_size) * num_packets;
  static const size_t total_frame_size    = data_size + sizeof(FrameHeader);
}

#endif /* INCLUDE_EXAMPLEDETECTORDEFINITIONS_H_ */
