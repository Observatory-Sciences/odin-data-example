"""
Created on 17th November 2025
:author: Alan Greer
"""
from datetime import datetime
import logging
import socket
import struct
import threading
import time
from odin.adapters.parameter_tree import ParameterTree


class ExampleDetectorController(object):
    # Class constants
    TIME_TICK = 0.1

    def __init__(self):
        logging.basicConfig(format='%(asctime)-15s %(message)s')
        self._log = logging.getLogger(".".join([__name__, self.__class__.__name__]))
        self._log.setLevel(logging.DEBUG)

        # Internal parameters
        self._config_frames = 0
        self._config_exposure_time = 1.0
        self._acquiring = False
        self._acquired_frames = 0
        self._update_time = datetime.now()
        self._udp_socket = None
        self._address = "localhost"
        self._port = 61649

        self.load_image()
        self.create_socket()

        # Parameter tree
        self._tree = {
            "config": {
                "frames": (self.get_config_frames, self.set_config_frames, {}),
                "exposure_time": (self.get_exposure_time, self.set_exposure_time, {}),
                "start": (lambda: 0, self.start, {}),
                "stop": (lambda: 0, self.stop, {}),
            },
            "status": {
                "acquiring": (lambda: self._acquiring, None, {}),
                "frames": (lambda: self._acquired_frames, None, {})
            }
        }
        self._params = ParameterTree(self._tree)

        # Set up acquisition thread
        self._acq_thread_running = True
        self._acq_lock = threading.Lock()
        self._acq_thread = threading.Thread(target=self.update_loop)
        self._acq_thread.start()

    def cleanup(self):
        self._acq_thread_running = False

    def get(self, path):
        return self._params.get(path)

    def set(self, path, data):
        self._params.set(path, data)

    def load_image(self):
        with open("./image.data", "rb") as f:
            self._image_bytes = f.read()

    def create_socket(self):
        # Create the UDP socket
        self._udp_socket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

    def set_config_frames(self, value):
        self._config_frames = value

    def get_config_frames(self):
        return self._config_frames
    
    def set_exposure_time(self, value):
        self._config_exposure_time = value

    def get_exposure_time(self):
        return self._config_exposure_time

    def start(self):
        # Take the lock
        with self._acq_lock:
            # Reset the frames to zero
            self._acquired_frames = 0
            # Set the detector to acquiring
            self._acquiring = True

    def stop(self):
        # Take the lock
        with self._acq_lock:
            # Turn off acquiring
            self._acquiring = False

    def update_loop(self):
        while self._acq_thread_running:
            # Execute at 1/TIME_TICK Hz
            time.sleep(ExampleDetectorController.TIME_TICK)
            # Take the lock
            with self._acq_lock:
                # Are we acquiring
                if self._acquiring:
                    # Have we waited long enough for a new frame
                    if (datetime.now() - self._update_time).total_seconds() >= self._config_exposure_time:
                        # Send the frame
                        print("Sending frame", self._acquired_frames)
                        self.send_frame()
                        # Increment the frame count
                        self._acquired_frames += 1
                        # Reset the timer
                        self._update_time = datetime.now()
                    # Check if the number of frames requested have been sent
                    if self._acquired_frames == self._config_frames:
                        # Turn off acquiring
                        self._acquiring = False

    def send_frame(self):
        image_lines = [self._image_bytes[i:i + 256] for i in range(0, len(self._image_bytes), 256)]
        packet = 0
        # Inject the frame number into the data

        for line in image_lines:
            # Create the header information
            header_bytes = struct.pack('<I', self._acquired_frames)
            header_bytes += struct.pack('<I', packet)
            header_bytes += struct.pack('<I', len(line)+12)
            # Create the full packet
            packet_bytes = header_bytes+line
            # Send the bytes over UDP
            bytes_sent = self._udp_socket.sendto(packet_bytes, (self._address, self._port))
            self._log.debug("Bytes sent for packet [%d]: %d", packet, bytes_sent)
            packet += 1


def main():
    """Run the odin-data client."""
    app = ExampleDetectorController()
    app.set_config_frames(2)
    app.start()
    time.sleep(10.0)


if __name__ == "__main__":
    main()
