// ImagerControl (should be built into Imager, because it will always
// be the same device).  The app doesn't have to use all of the
// functions if they don't want to.
// XXX Client can sent request for only subregion of image to be sent
//     Server may ignore this message.
// XXX Server sets region back to total region when last connection closed.
// XXX Client can request a frame rate from the server.  This is passed on
//     to the server code as a handled message.  Server should reset to the
//     default when the last connection is closed.
// XXX Binning
// XXX integration times
// XXX Which data sets to send (nano)

// ImagerPose (may be a separate physical device from the imager)
// XXX Lets client request new pose for imager

// XXX When transcoding to a lower-bitcount resolution, should we
// adjust the scale and offset to make best use of the bits?  Perhaps
// a local and a global scale and offset?

#ifndef	VRPN_IMAGER_H
#define	VRPN_IMAGER_H
#include <string.h>	// For memcpy()
#include  "vrpn_Connection.h"
#include  "vrpn_BaseClass.h"

const unsigned vrpn_IMAGER_MAX_CHANNELS = 100;

/// Set of constants to tell how many points you can put into a region
/// depending on the type you are putting in there.  Useful for senders
/// to know how large of a chunk they can send at once.
const unsigned vrpn_IMAGER_MAX_REGIONu8 = (vrpn_CONNECTION_TCP_BUFLEN - 8*sizeof(vrpn_int16) - 6*sizeof(vrpn_int32))/sizeof(vrpn_uint8);
const unsigned vrpn_IMAGER_MAX_REGIONu16 = (vrpn_CONNECTION_TCP_BUFLEN - 8*sizeof(vrpn_int16) - 6*sizeof(vrpn_int32))/sizeof(vrpn_uint16);
const unsigned vrpn_IMAGER_MAX_REGIONu12in16 = vrpn_IMAGER_MAX_REGIONu16;
const unsigned vrpn_IMAGER_MAX_REGIONf32 = (vrpn_CONNECTION_TCP_BUFLEN - 8*sizeof(vrpn_int16) - 6*sizeof(vrpn_int32))/sizeof(vrpn_float32);

/// Holds the description needed to convert from raw data to values for a channel
class VRPN_API vrpn_Imager_Channel {
  friend class vrpn_Imager_Remote;    // provides access to compression status
  friend class vrpn_Imager_Server;    // provides access to compression status
  friend class vrpn_Imager_Stream_Buffer; // provides access to buffer/unbuffer
public:
  vrpn_Imager_Channel(void) { name[0] = '\0'; units[0] = '\0'; minVal = maxVal = 0.0;
      scale = 1; offset = 0; d_compression = NONE; };

  cName	name;			//< Name of the data set stored in this channel
  cName	units;			//< Units for the data set stored in this channel
  vrpn_float32	minVal, maxVal; //< Range of possible values for pixels in this channel
  vrpn_float32	offset, scale;	//< Values in units are (raw_values * scale) + offset

protected:
  // The following methods are here for the derived classes and are not relevant
  // to user code.
  inline  bool	buffer(char **insertPt, vrpn_int32 *buflen) const {
    if (vrpn_buffer(insertPt, buflen, minVal) ||
        vrpn_buffer(insertPt, buflen, maxVal) ||
        vrpn_buffer(insertPt, buflen, offset) ||
        vrpn_buffer(insertPt, buflen, scale) ||
	vrpn_buffer(insertPt, buflen, (vrpn_uint32)d_compression) ||
	vrpn_buffer(insertPt, buflen, name, sizeof(name)) ||
	vrpn_buffer(insertPt, buflen, units, sizeof(units)) ) {
      return false;
    } else {
      return true;
    }
  }

  inline  bool	unbuffer(const char **buffer) {
    vrpn_uint32	compression;
    if (vrpn_unbuffer(buffer, &minVal) ||
        vrpn_unbuffer(buffer, &maxVal) ||
        vrpn_unbuffer(buffer, &offset) ||
        vrpn_unbuffer(buffer, &scale) ||
        vrpn_unbuffer(buffer, &compression) ||
	vrpn_unbuffer(buffer, name, sizeof(name)) ||
	vrpn_unbuffer(buffer, units, sizeof(units)) ) {
      return false;
    } else {
      d_compression = (ChannelCompression)compression;
      return true;
    }
  }

  typedef enum { NONE = 0 } ChannelCompression;
  ChannelCompression	d_compression;
};

/// Base class for Imager class
class VRPN_API vrpn_Imager: public vrpn_BaseClass {
public:
  vrpn_Imager(const char *name, vrpn_Connection *c = NULL);

  // Data member accessors.
  vrpn_int32	nRows(void) const { return d_nRows; };
  vrpn_int32	nCols(void) const { return d_nCols; };
  vrpn_int32	nDepth(void) const { return d_nDepth; };
  vrpn_int32	nChannels(void) const { return d_nChannels; };

protected:
  vrpn_int32	d_nRows;	//< Number of rows in the image
  vrpn_int32	d_nCols;	//< Number of columns in the image
  vrpn_int32	d_nDepth;	//< Number of depth stacks in the image
  vrpn_int32	d_nChannels;	//< Number of image data channels
  vrpn_Imager_Channel d_channels[vrpn_IMAGER_MAX_CHANNELS];

  virtual int register_types(void);
  vrpn_int32	d_description_m_id;	//< ID of the message type describing the range and channels
  vrpn_int32	d_begin_frame_m_id;	//< ID of the message type describing the start of a region
  vrpn_int32	d_end_frame_m_id;	//< ID of the message type describing the start of a region
  vrpn_int32	d_discarded_frames_m_id;//< ID of the message type describing the discarding of one or more regions
  vrpn_int32	d_throttle_frames_m_id;	//< ID of the message type requesting throttling of sending.
  vrpn_int32	d_regionu8_m_id;	//< ID of the message type describing a region with 8-bit unsigned entries
  vrpn_int32	d_regionu12in16_m_id;   //< ID of the message type describing a region with 12-bit unsigned entries packed in 16 bits
  vrpn_int32	d_regionu16_m_id;	//< ID of the message type describing a region with 16-bit unsigned entries
  vrpn_int32	d_regionf32_m_id;	//< ID of the message type describing a region with 32-bit float entries
};

class VRPN_API vrpn_Imager_Server: public vrpn_Imager {
public:
  vrpn_Imager_Server(const char *name, vrpn_Connection *c,
		     vrpn_int32 nCols, vrpn_int32 nRows, vrpn_int32 nDepth = 1);

  /// Add a channel to the server, returns index of the channel or -1 on failure.
  int	add_channel(const char *name, const char *units = "unsigned8bit",
		    vrpn_float32 minVal = 0, vrpn_float32 maxVal = 255,
		    vrpn_float32 scale = 1, vrpn_float32 offset = 0);

  /// Servers must send begin/end frame pairs around contiguous sections of the image
  // to provide hints to the client about when to refresh displays and such.
  // If they can determine when frames are missed, they should also send a
  // description of missed frames, telling how many are skipped (default of
  // zero means "some but don't know how many").
  bool	send_begin_frame(const vrpn_uint16 cMin, const vrpn_uint16 cMax,
			 const vrpn_uint16 rMin, const vrpn_uint16 rMax,
			 const vrpn_uint16 dMin = 0, const vrpn_uint16 dMax = 0,
			 const struct timeval *time = NULL);
  bool	send_end_frame(const vrpn_uint16 cMin, const vrpn_uint16 cMax,
		       const vrpn_uint16 rMin, const vrpn_uint16 rMax,
			 const vrpn_uint16 dMin = 0, const vrpn_uint16 dMax = 0,
		       const struct timeval *time = NULL);
  bool	send_discarded_frames(const vrpn_uint16 count = 0,
			 const struct timeval *time = NULL);

  /// Pack and send the region as efficiently as possible; strides are in steps of the element being sent.
  // These functions each take a pointer to the base of the image to be sent: its [0,0] element.
  // If rows are being inverted, then we need to know how many rows there are in the total image.
  bool	send_region_using_base_pointer(vrpn_int16 chanIndex, vrpn_uint16 cMin, vrpn_uint16 cMax,
		    vrpn_uint16 rMin, vrpn_uint16 rMax, const vrpn_uint8 *data,
		    vrpn_uint32	colStride, vrpn_uint32 rowStride,
		    vrpn_uint16 nRows = 0, bool invert_rows = false,
		    vrpn_uint32 depthStride = 0, vrpn_uint16 dMin = 0, vrpn_uint16 dMax = 0,
		    const struct timeval *time = NULL);
  bool	send_region_using_base_pointer(vrpn_int16 chanIndex, vrpn_uint16 cMin, vrpn_uint16 cMax,
		    vrpn_uint16 rMin, vrpn_uint16 rMax, const vrpn_uint16 *data,
		    vrpn_uint32	colStride, vrpn_uint32 rowStride,
		    vrpn_uint16 nRows = 0, bool invert_rows = false,
		    vrpn_uint32 depthStride = 0, vrpn_uint16 dMin = 0, vrpn_uint16 dMax = 0,
		    const struct timeval *time = NULL);
  bool	send_region_using_base_pointer(vrpn_int16 chanIndex, vrpn_uint16 cMin, vrpn_uint16 cMax,
		    vrpn_uint16 rMin, vrpn_uint16 rMax, const vrpn_float32 *data,
		    vrpn_uint32	colStride, vrpn_uint32 rowStride,
		    vrpn_uint16 nRows = 0, bool invert_rows = false,
		    vrpn_uint32 depthStride = 0, vrpn_uint16 dMin = 0, vrpn_uint16 dMax = 0, const struct timeval *time = NULL);

  /// Pack and send the region as efficiently as possible; strides are in steps of the element being sent.
  // These functions each take a pointer to the first of the data values to be sent.  This is a
  // pointer to the [cMin, rMin] element of the image to be sent.  Note that if the Y value is inverted,
  // this will NOT be a pointer to the beginning of the data block, but rather the the beginning of
  // the last line in the data block.  Note that rowStride will be less than the number of rows in the
  // whole image if the data is tightly packed into a block and the region does not cover all columns.
  bool	send_region_using_first_pointer(vrpn_int16 chanIndex, vrpn_uint16 cMin, vrpn_uint16 cMax,
		    vrpn_uint16 rMin, vrpn_uint16 rMax, const vrpn_uint8 *data,
		    vrpn_uint32	colStride, vrpn_uint32 rowStride,
		    vrpn_uint16 nRows = 0, bool invert_rows = false,
		    vrpn_uint32 depthStride = 0, vrpn_uint16 dMin = 0, vrpn_uint16 dMax = 0,
		    const struct timeval *time = NULL);
  bool	send_region_using_first_pointer(vrpn_int16 chanIndex, vrpn_uint16 cMin, vrpn_uint16 cMax,
		    vrpn_uint16 rMin, vrpn_uint16 rMax, const vrpn_uint16 *data,
		    vrpn_uint32	colStride, vrpn_uint32 rowStride,
		    vrpn_uint16 nRows = 0, bool invert_rows = false,
		    vrpn_uint32 depthStride = 0, vrpn_uint16 dMin = 0, vrpn_uint16 dMax = 0,
		    const struct timeval *time = NULL);
  bool	send_region_using_first_pointer(vrpn_int16 chanIndex, vrpn_uint16 cMin, vrpn_uint16 cMax,
		    vrpn_uint16 rMin, vrpn_uint16 rMax, const vrpn_float32 *data,
		    vrpn_uint32	colStride, vrpn_uint32 rowStride,
		    vrpn_uint16 nRows = 0, bool invert_rows = false,
		    vrpn_uint32 depthStride = 0, vrpn_uint16 dMin = 0, vrpn_uint16 dMax = 0,
		    const struct timeval *time = NULL);

  /// Set the resolution to a different value than it had been before.  Returns true on success.
  bool	set_resolution(vrpn_int32 nCols, vrpn_int32 nRows, vrpn_int32 nDepth = 1);

  /// Sends a description of the imager so the remote can process the region messages
  bool	send_description(void);

  /// Handle baseclass ping/pong messages
  virtual void	mainloop(void);

protected:
  bool	      d_description_sent;   //< Has the description message been sent?
  vrpn_int32  d_frames_to_send;	    //< Set to -1 if continuous, zero or positive tells how many to send and then start dropping
  vrpn_uint16 d_dropped_due_to_throttle;  //< Number of frames dropped due to the throttle request

  // This method makes sure we send a description whenever we get a ping from
  // a client object.
  static  int VRPN_CALLBACK handle_ping_message(void *userdata, vrpn_HANDLERPARAM p);

  // This method handles requests to throttle the number of frames.
  static  int VRPN_CALLBACK handle_throttle_message(void *userdata, vrpn_HANDLERPARAM p);
  static  int VRPN_CALLBACK handle_last_drop_message(void *userdata, vrpn_HANDLERPARAM p);
};

class VRPN_API	vrpn_ImagerPose: public vrpn_BaseClass {
public:
  vrpn_ImagerPose(const char *name, vrpn_Connection *c = NULL);

  /// Returns the origin of the coordinate system,
  // the location of the corner of the (0,0,0) pixel.  Note that
  // the pixel coordinate is centered in that pixel, but that the
  // pixel extends a half-pixel into the "negative" coordinates.
  void	get_origin(vrpn_float64 *origin) const {
    memcpy(origin, d_origin, sizeof(d_origin));
  }

  /// This is the total span of the image in columns;
  // it is how far and in what direction to go from the origin
  // of the image to one pixel past the pixel at the end of
  // the column that (0,0,0) is in: this is the total image
  // width.
  void	get_dCol(vrpn_float64 *dCol) const {
    memcpy(dCol, d_dCol, sizeof(d_dCol));
  }

  /// This is the total span of the image in rows;
  // it is how far and in what direction to go from the origin
  // of the image to one pixel past the pixel at the end of
  // the row that (0,0,0) is in: this is the total image height.
  void	get_dRow(vrpn_float64 *dRow) const {
    memcpy(dRow, d_dRow, sizeof(d_dRow));
  }

  /// This is the total span of the image in depth;
  // it is how far and in what direction to go from the origin
  // of the image to one pixel past the pixel at the end of
  // the depth pixel that (0,0,0) is in: this is the total
  // image depth.
  void	get_dDepth(vrpn_float64 *dDepth) const {
    memcpy(dDepth, d_dDepth, sizeof(d_dDepth));
  }

  /// This will return the location of the center of the specified
  // pixel within the image, assuming that the image covers the
  // space described by this imagerpose.  Note that none of the pixel
  // centers will be at the end of the space, except where the image
  // has no dimension (Z for a 2D image).  Returns false if there is
  // a problem (coordinates out of bounds).
  bool	compute_pixel_center(vrpn_float64 *center, const vrpn_Imager &image,
    vrpn_uint16 col, vrpn_uint16 row, vrpn_uint16 depth = 0);

protected:
  vrpn_float64	d_origin[3];	//< Origin, pixel (0,0,0) in meters
  vrpn_float64	d_dCol[3];	//< End of first columne in coordinate system in meters
  vrpn_float64	d_dRow[3];	//< End of first row in coordinate system in meters
  vrpn_float64	d_dDepth[3];	//< End of depth in coordinate system in meters

  virtual int register_types(void);
  vrpn_int32  d_description_m_id;	//< ID of the message type describing the range and channels
};

class VRPN_API	vrpn_ImagerPose_Server: public vrpn_ImagerPose {
public:

  vrpn_ImagerPose_Server(const char *name, 
    const vrpn_float64 origin[3], const vrpn_float64 dCol[3],
    const vrpn_float64 dRow[3], const vrpn_float64 *dDepth = NULL,
    vrpn_Connection *c = NULL);

  /// Set the range or units.  Return true on success.
  bool	set_range(const vrpn_float64 origin[3], const vrpn_float64 dCol[3],
    const vrpn_float64 dRow[3], const vrpn_float64 *dDepth = NULL);

  /// Sends a description of the imager so the remote can process the region messages
  bool	send_description(void);

  /// Handle baseclass ping/pong messages
  virtual void	mainloop(void);

protected:
  // This method makes sure we send a description whenever we get a ping from
  // a client object.
  static  int VRPN_CALLBACK handle_ping_message(void *userdata, vrpn_HANDLERPARAM p);
};

//------------------------------------------------------------------------------
// Users deal with things below this line.

//------------------------------------------------------------------------------
// Imager_Remote is used for passing image values (pixels), converting them
// to physical units, and saying when regions are started and finished.

const vrpn_uint16 vrpn_IMAGER_VALTYPE_UNKNOWN	  = 0;
const vrpn_uint16 vrpn_IMAGER_VALTYPE_UINT8	  = 1;
//XXX Bad idea -- do not do this! const vrpn_uint16 vrpn_IMAGER_VALTYPE_UINT8RGB	  = 2;	// Placeholder
//XXX Bad idea -- do not do this! const vrpn_uint16 vrpn_IMAGER_VALTYPE_UINT8BGR	  = 3;	// Placeholder
const vrpn_uint16 vrpn_IMAGER_VALTYPE_UINT16	  = 4;
const vrpn_uint16 vrpn_IMAGER_VALTYPE_UINT12IN16  = 5;
const vrpn_uint16 vrpn_IMAGER_VALTYPE_FLOAT32	  = 6;

class VRPN_API	vrpn_Imager_Region;

typedef struct _vrpn_IMAGERREGIONCB {
  struct timeval		msg_time; //< Timestamp of the region data's change
  const vrpn_Imager_Region	*region;  //< New region of the image
} vrpn_IMAGERREGIONCB;

typedef void (VRPN_CALLBACK *vrpn_IMAGERREGIONHANDLER) (void * userdata,
					  const vrpn_IMAGERREGIONCB info);
// There is no data in the description callback other than the time; the
// data members for the class will have been filled in, so the client should
// call nRows() and other functions to read the new values.
typedef void (VRPN_CALLBACK *vrpn_IMAGERDESCRIPTIONHANDLER) (void * userdata,
					       const struct timeval msg_time);

typedef struct _vrpn_IMAGERBEGINFRAMECB {
  struct timeval  msg_time;	//< Timestamp of the begin-frame message
  vrpn_uint16	  rMin;		//< Minimum row in the frame
  vrpn_uint16	  rMax;		//< Maximum row in the frame
  vrpn_uint16	  cMin;		//< Minimum column in the frame
  vrpn_uint16	  cMax;		//< Maximum column in the frame
  vrpn_uint16	  dMin;		//< Minimum depth in the frame
  vrpn_uint16	  dMax;		//< Maximum depth in the frame
} vrpn_IMAGERBEGINFRAMECB;

typedef struct _vrpn_IMAGERENDFRAMECB {
  struct timeval  msg_time;	//< Timestamp of the end-frame message
  vrpn_uint16	  rMin;		//< Minimum row in the frame
  vrpn_uint16	  rMax;		//< Maximum row in the frame
  vrpn_uint16	  cMin;		//< Minimum column in the frame
  vrpn_uint16	  cMax;		//< Maximum column in the frame
  vrpn_uint16	  dMin;		//< Minimum depth in the frame
  vrpn_uint16	  dMax;		//< Maximum depth in the frame
} vrpn_IMAGERENDFRAMECB;

typedef struct _vrpn_IMAGERDISCARDEDFRAMESCB {
  struct timeval  msg_time;	//< Timestamp of the begin-frame message
  vrpn_uint16	  count;	//< Number of discarded frames (0 means "1 or more")
} vrpn_IMAGERDISCARDEDFRAMESCB;

typedef void (VRPN_CALLBACK *vrpn_IMAGERBEGINFRAMEHANDLER) (void * userdata,
					  const vrpn_IMAGERBEGINFRAMECB info);
typedef void (VRPN_CALLBACK *vrpn_IMAGERENDFRAMEHANDLER) (void * userdata,
					  const vrpn_IMAGERENDFRAMECB info);
typedef void (VRPN_CALLBACK *vrpn_IMAGERDISCARDEDFRAMESHANDLER) (void * userdata,
					  const vrpn_IMAGERDISCARDEDFRAMESCB info);


/// Helper function to convert data for a sub-region of one channel of
// the image.  This is passed to the user callback handler and aids in
// getting values out of the buffer.  The region is only valid during
// the actual callback handler, so users should not store pointers to
// it for later use.
class VRPN_API vrpn_Imager_Region {
  friend class VRPN_API vrpn_Imager_Remote;
  friend void VRPN_CALLBACK java_vrpn_handle_region_change( void * userdata, const vrpn_IMAGERREGIONCB info );

public:
  vrpn_Imager_Region(void) { d_chanIndex = -1; d_rMin = d_rMax = d_cMin = d_cMax = 0; 
				 d_valBuf = NULL; d_valType = vrpn_IMAGER_VALTYPE_UNKNOWN;
				 d_valid = false; }

  /// Returns the number of values in the region.
  inline vrpn_uint32 getNumVals( ) const {
    if (!d_valid) { return 0;
    } else { return ( d_rMax - d_rMin + 1 ) * ( d_cMax - d_cMin + 1 ); }
  }

  /// Reads pixel from the region with no scale and offset applied to the value.  Not
  /// the most efficient way to read the pixels out -- use the block read routines.
  inline  bool	read_unscaled_pixel(vrpn_uint16 c, vrpn_uint16 r, vrpn_uint8 &val, vrpn_uint16 d=0) const {
    if ( !d_valid || (c < d_cMin) || (c > d_cMax) || (r < d_rMin) || (r > d_rMax) ) {
      fprintf(stderr, "vrpn_Imager_Region::read_unscaled_pixel(): Invalid region or out of range\n");
      return false;
    } else {
      if (d_valType != vrpn_IMAGER_VALTYPE_UINT8) {
	fprintf(stderr, "XXX vrpn_Imager_Region::read_unscaled_pixel(): Transcoding not implemented yet\n");
	return false;
      } else {
	// The data is packed in with column varying fastest, row varying next, and depth
	// varying slowest.  Depth steps are therefore the largest steps.
	val = ((vrpn_uint8 *)d_valBuf)[(c - d_cMin) + (d_cMax - d_cMin+1)*( (r - d_rMin) + (d - d_dMin)*(d_rMax - d_rMin+1) )];
      }
    }
    return true;
  }

  /// Reads pixel from the region with no scale and offset applied to the value.  Not
  // the most efficient way to read the pixels out -- use the block read routines.
  inline  bool	read_unscaled_pixel(vrpn_uint16 c, vrpn_uint16 r, vrpn_uint16 &val, vrpn_uint16 d=0) const {
    if ( !d_valid || (d < d_dMin) || (d > d_dMax) || (c < d_cMin) || (c > d_cMax) || (r < d_rMin) || (r > d_rMax) ) {
      fprintf(stderr, "vrpn_Imager_Region::read_unscaled_pixel(): Invalid region or out of range\n");
      return false;
    } else {
      if ((d_valType != vrpn_IMAGER_VALTYPE_UINT16) && (d_valType != vrpn_IMAGER_VALTYPE_UINT12IN16) ) {
	fprintf(stderr, "XXX vrpn_Imager_Region::read_unscaled_pixel(): Transcoding not implemented yet\n");
	return false;
      } else if (vrpn_big_endian) {
	fprintf(stderr, "XXX vrpn_Imager_Region::read_unscaled_pixel(): Not implemented on big-endian yet\n");
	return false;
      } else {
	// The data is packed in with column varying fastest, row varying next, and depth
	// varying slowest.  Depth steps are therefore the largest steps.
	val = ((vrpn_uint16 *)d_valBuf)[(c - d_cMin) + (d_cMax - d_cMin+1)*( (r - d_rMin) + (d - d_dMin)*(d_rMax - d_rMin+1) )];
      }
    }
    return true;
  }

  /// Reads pixel from the region with no scale and offset applied to the value.  Not
  // the most efficient way to read the pixels out -- use the block read routines.
  inline  bool	read_unscaled_pixel(vrpn_uint16 c, vrpn_uint16 r, vrpn_float32 &val, vrpn_uint16 d=0) const {
    if ( !d_valid || (d < d_dMin) || (d > d_dMax) || (c < d_cMin) || (c > d_cMax) || (r < d_rMin) || (r > d_rMax) ) {
      fprintf(stderr, "vrpn_Imager_Region::read_unscaled_pixel(): Invalid region or out of range\n");
      return false;
    } else {
      if (d_valType != vrpn_IMAGER_VALTYPE_FLOAT32) {
	fprintf(stderr, "XXX vrpn_Imager_Region::read_unscaled_pixel(): Transcoding not implemented yet\n");
	return false;
      } else if (vrpn_big_endian) {
	fprintf(stderr, "XXX vrpn_Imager_Region::read_unscaled_pixel(): Not implemented on big-endian yet\n");
	return false;
      } else {
	// The data is packed in with column varying fastest, row varying next, and depth
	// varying slowest.  Depth steps are therefore the largest steps.
	val = ((vrpn_float32 *)d_valBuf)[(c - d_cMin) + (d_cMax - d_cMin+1)*( (r - d_rMin) + (d - d_dMin)*(d_rMax - d_rMin+1) )];
      }
    }
    return true;
  }

  // Bulk read routines to copy the whole region right into user structures as
  // efficiently as possible.
  bool	decode_unscaled_region_using_base_pointer(vrpn_uint8 *data,
    vrpn_uint32 colStride, vrpn_uint32 rowStride, vrpn_uint32 depthStride = 0,
    vrpn_uint16 nRows = 0, bool invert_rows = false, unsigned repeat = 1) const;
  // This routine also reads 12-bits-in-16-bit values.
  bool	decode_unscaled_region_using_base_pointer(vrpn_uint16 *data,
    vrpn_uint32 colStride, vrpn_uint32 rowStride, vrpn_uint32 depthStride = 0,
    vrpn_uint16 nRows = 0, bool invert_rows = false, unsigned repeat = 1) const;
  bool	decode_unscaled_region_using_base_pointer(vrpn_float32 *data,
    vrpn_uint32 colStride, vrpn_uint32 rowStride, vrpn_uint32 depthStride = 0,
    vrpn_uint16 nRows = 0, bool invert_rows = false, unsigned repeat = 1) const;

  // XXX Add routines to read scaled pixels.  Clamp values.

  vrpn_int16  d_chanIndex;	//< Which channel this region holds data for 
  vrpn_uint16 d_rMin, d_rMax;	//< Range of indices for the rows
  vrpn_uint16 d_cMin, d_cMax;	//< Range of indices for the columns
  vrpn_uint16 d_dMin, d_dMax;	//< Range of indices for the depth

protected:
  const	void  *d_valBuf;	//< Pointer to the buffer of values
  vrpn_uint16 d_valType;	//< Type of the values in the buffer
  bool	      d_valid;		//< Tells whether the helper can be used.
};

/// This is the class users deal with: it tells the format and the region data when it arrives.
class VRPN_API vrpn_Imager_Remote: public vrpn_Imager {
public:
  vrpn_Imager_Remote(const char *name, vrpn_Connection *c = NULL);

  /// Register a handler for when new data arrives (can look up info in object when this happens)
  virtual int register_region_handler(void *userdata, vrpn_IMAGERREGIONHANDLER handler) {
    return d_region_list.register_handler(userdata, handler);
  };
  virtual int unregister_region_handler(void *userdata, vrpn_IMAGERREGIONHANDLER handler) {
    return d_region_list.unregister_handler(userdata, handler);
  }

  /// Register a handler for when the object's description changes (if desired).
  virtual int register_description_handler(void *userdata, vrpn_IMAGERDESCRIPTIONHANDLER handler) {
    return d_description_list.register_handler(userdata, handler);
  };
  virtual int unregister_description_handler(void *userdata, vrpn_IMAGERDESCRIPTIONHANDLER handler) {
    return d_description_list.unregister_handler(userdata, handler);
  }

  /// Register a handler for frame beginning (if the application cares)
  virtual int register_begin_frame_handler(void *userdata, vrpn_IMAGERBEGINFRAMEHANDLER handler) {
    return d_begin_frame_list.register_handler(userdata, handler);
  };
  virtual int unregister_begin_frame_handler(void *userdata, vrpn_IMAGERBEGINFRAMEHANDLER handler) {
    return d_begin_frame_list.unregister_handler(userdata, handler);
  }

  /// Register a handler for frame end (if the application cares)
  virtual int register_end_frame_handler(void *userdata, vrpn_IMAGERENDFRAMEHANDLER handler) {
    return d_end_frame_list.register_handler(userdata, handler);
  };
  virtual int unregister_end_frame_handler(void *userdata, vrpn_IMAGERENDFRAMEHANDLER handler) {
    return d_end_frame_list.unregister_handler(userdata, handler);
  }

  /// Register a handler for discarded frame notifications (if the application cares)
  virtual int register_discarded_frames_handler(void *userdata, vrpn_IMAGERDISCARDEDFRAMESHANDLER handler) {
    return d_discarded_frames_list.register_handler(userdata, handler);
  };
  virtual int unregister_discarded_frames_handler(void *userdata, vrpn_IMAGERDISCARDEDFRAMESHANDLER handler) {
    return d_discarded_frames_list.unregister_handler(userdata, handler);
  }

  /// Request that the server send at most N more frames until a new request is sent.
  // This is used to throttle senders that are incurring lots of latency by filling
  // the network with packets and blocking.  The next request for "N" will add onto
  // the request.  Sending "-1" means to send continuously as fast as possible,
  // which is the default.
  virtual bool throttle_sender(vrpn_int32 N);

  /// XXX It could be nice to let the user specify separate callbacks for
  // region size changed (which would be called only if the description had
  // a different region size than the last time, and also the first time it
  // is called) and channel changes (which would require keeping a copy of
  // the old and diffing when a new description came in).  Also, the interace
  // could hook different callbacks for different channels IDs to let the
  // Imager do the work of sorting out any mapping changes and keeping track
  // of which channel is handled by which callback -- like the Tracker and its
  // sensors.  This should happen by name, rather than by index.  It might be
  // nice to provide a delete callback when a channel is removed and an add
  // callback when a channel is added as well, and a change callback if the
  // name, units, scale or offset change.

  /// Call this each time through the program's main loop
  virtual void	mainloop(void);

  /// Accessors for the member variables: can be queried in the handler for object changes
  const vrpn_Imager_Channel *channel(unsigned chanNum) const;

  /// have we gotten a description message yet?
  bool is_description_valid() {  return d_got_description;  }

protected:
  bool	  d_got_description;	//< Have we gotten a description yet?
  // Lists to keep track of registered user handlers.
  vrpn_Callback_List<struct timeval>		    d_description_list;
  vrpn_Callback_List<vrpn_IMAGERREGIONCB>	    d_region_list;
  vrpn_Callback_List<vrpn_IMAGERBEGINFRAMECB>	    d_begin_frame_list;
  vrpn_Callback_List<vrpn_IMAGERENDFRAMECB>	    d_end_frame_list;
  vrpn_Callback_List<vrpn_IMAGERDISCARDEDFRAMESCB>  d_discarded_frames_list;

  /// Handler for region update message from the server.
  static int VRPN_CALLBACK handle_region_message(void *userdata, vrpn_HANDLERPARAM p);

  /// Handler for resolution and channel list message from the server.
  static int VRPN_CALLBACK handle_description_message(void *userdata, vrpn_HANDLERPARAM p);

  /// Handler for connection dropped message
  static int VRPN_CALLBACK handle_connection_dropped_message(void *userdata, vrpn_HANDLERPARAM p);

  /// Handler for begin-frame message from the server.
  static int VRPN_CALLBACK handle_begin_frame_message(void *userdata, vrpn_HANDLERPARAM p);

  /// Handler for end-frame message from the server.
  static int VRPN_CALLBACK handle_end_frame_message(void *userdata, vrpn_HANDLERPARAM p);

  /// Handler for discarded-frames message from the server.
  static int VRPN_CALLBACK handle_discarded_frames_message(void *userdata, vrpn_HANDLERPARAM p);
};

//------------------------------------------------------------------------------
// ImagerPose_Remote deals with the physical size and location the pixels in
// an image.

typedef void (VRPN_CALLBACK *vrpn_IMAGERPOSEDESCRIPTIONHANDLER) (void * userdata,
					       const struct timeval msg_time);

class VRPN_API vrpn_ImagerPose_Remote: public vrpn_ImagerPose {
public:
  vrpn_ImagerPose_Remote(const char *name, vrpn_Connection *c = NULL);

  /// Register a handler for when the object's description changes (if desired)
  virtual int register_description_handler(void *userdata, vrpn_IMAGERDESCRIPTIONHANDLER handler) {
    return d_description_list.register_handler(userdata, handler);
  };
  virtual int unregister_description_handler(void *userdata, vrpn_IMAGERDESCRIPTIONHANDLER handler) {
    return d_description_list.unregister_handler(userdata, handler);
  }

  /// Call this each time through the program's main loop
  virtual void	mainloop(void);

protected:
  // Lists to keep track of registered user handlers.
  vrpn_Callback_List<struct timeval>  d_description_list;

  /// Handler for resolution and channel list message from the server.
  static int VRPN_CALLBACK handle_description_message(void *userdata, vrpn_HANDLERPARAM p);
};

#endif
