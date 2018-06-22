/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file vrpnClient.I
 * @author jason
 * @date 2000-08-04
 */

/**
 * Returns the name of the server as passed to the VrpnClient constructor.
 */
INLINE const std::string &VrpnClient::
get_server_name() const {
  return _server_name;
}

/**
 * Returns true if everything seems to be kosher with the server (even if
 * there is no connection), or false otherwise.
 */
INLINE bool VrpnClient::
is_valid() const {
  return (_connection->doing_okay() != 0);
}

/**
 * Returns true if the connection is established successfully, false
 * otherwise.
 */
INLINE bool VrpnClient::
is_connected() const {
  return (_connection->connected() != 0);
}


/**
 * Little inline function to convert a struct timeval to only seconds
 */
INLINE double VrpnClient::
convert_to_secs(struct timeval msg_time) {
  return (double)(msg_time.tv_sec) + (double)msg_time.tv_usec * 0.000001;
}




#if 0

/**
 *
 */
INLINE VrpnClient::
VrpnClient(const std::string &server) :
  ClientBase(server)
{
  _connection = vrpn_get_connection_by_name(server.c_str());
}

/**
 * Stores the latest position information as sent by the tracker (for the
 * particular sensor we have interest in)
 */
INLINE void VrpnClient::
tracker_position(const std::string &tracker, const vrpn_TRACKERCB info) {
  double ptime = convert_to_secs(info.msg_time);
  LPoint3 pos(info.pos[0], info.pos[1], info.pos[2]);
  LVector4 pquat(info.quat[0], info.quat[1], info.quat[2], info.quat[3]);

  push_tracker_position(tracker, info.sensor, ptime, pos, pquat);
}

/**
 * Stores the latest velocity information as sent by the tracker (for the
 * particular sensor we have interest in)
 */
INLINE void VrpnClient::
tracker_velocity(const std::string &tracker, const vrpn_TRACKERVELCB info) {
  double vtime = convert_to_secs(info.msg_time);
  LPoint3 vel(info.vel[0], info.vel[1], info.vel[2]);
  LVector4 vquat(info.vel_quat[0], info.vel_quat[1],
                  info.vel_quat[2], info.vel_quat[3]);
  PN_stdfloat dt = info.vel_quat_dt;

  push_tracker_velocity(tracker, info.sensor, vtime, vel, vquat, dt);
}
/**
 * Stores the latest acceleration information as sent by the tracker (for the
 * particular sensor we have interest in)
 */
INLINE void VrpnClient::
tracker_acceleration(const std::string &tracker, const vrpn_TRACKERACCCB info) {
  double atime = convert_to_secs(info.msg_time);
  LPoint3 acc(info.acc[0], info.acc[1], info.acc[2]);
  LVector4 aquat(info.acc_quat[0], info.acc_quat[1],
                  info.acc_quat[2], info.acc_quat[3]);
  PN_stdfloat dt = info.acc_quat_dt;

  push_tracker_acceleration(tracker, info.sensor, atime, acc, aquat, dt);
}

/**
 * Stores the latest information as sent by the analog device
 */
INLINE void VrpnClient::
analog(const std::string &analog, const vrpn_ANALOGCB info) {
  double atime = convert_to_secs(info.msg_time);

  push_analog(analog, atime, info.channel, info.num_channel);
}

/**
 * Stores the latest button pressed information as sent by the button
 */
INLINE void VrpnClient::
button(const std::string &button, const vrpn_BUTTONCB info) {
  double btime = convert_to_secs(info.msg_time);

  push_button(button, btime, info.button, info.state);
}

/**
 * Stores the latest change information as sent by the dial
 */
INLINE void VrpnClient::
dial(const std::string &dial, const vrpn_DIALCB info) {
  double dtime = convert_to_secs(info.msg_time);

  push_dial(dial, dtime, info.dial, info.change);
}

#endif
