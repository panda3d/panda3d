#include "pandabase.h"
#ifdef HAVE_CV
#include "texture.h"
#include "aviTexture.h"
#include <stdio.h>




////////////////////////////////////////////////////////////////////
//     Function: AviTexture::AviTexture
//       Access: Published
//  Description: Sets up the texture to read frames from a camera
////////////////////////////////////////////////////////////////////
AviTexture::AviTexture()
{
  _isCamera=true;
  _capture = cvCaptureFromCAM(0);
  _buf=0;
  _magicNum=0;
  _time=0;
  _fps=30;
  _total_frames=0;
  _current_frame=0;
  
  if(_capture)
  {
    cvSetCaptureProperty(_capture,CV_CAP_PROP_FPS,_fps);
    _width=cvGetCaptureProperty(_capture,CV_CAP_PROP_FRAME_WIDTH);
    _height=cvGetCaptureProperty(_capture,CV_CAP_PROP_FRAME_HEIGHT);
    if(_width<_height)
      gen_tex(_width);
    else
      gen_tex(_height);
  }             
}


////////////////////////////////////////////////////////////////////
//     Function: AviTexture::AviTexture
//       Access: Published
//  Description: Defines the texture as a movie texture. 
//               TODO: Make this search the panda paths
////////////////////////////////////////////////////////////////////
AviTexture::AviTexture(const string &filename)
{
    _isCamera=false;
    _capture = cvCaptureFromFile(filename.c_str());
    _buf=0;
    _magicNum=0; 
    _current_frame=0;   
    if(_capture)
    {
      _fps=cvGetCaptureProperty(_capture,CV_CAP_PROP_FPS);
      _total_frames=cvGetCaptureProperty(_capture,CV_CAP_PROP_FRAME_COUNT);
      _width=cvGetCaptureProperty(_capture,CV_CAP_PROP_FRAME_WIDTH);
      _height=cvGetCaptureProperty(_capture,CV_CAP_PROP_FRAME_HEIGHT);
      if(_width<_height)
	gen_tex(_width);
      else
	gen_tex(_height);
    }             
}

////////////////////////////////////////////////////////////////////
//     Function: AviTexture::~AviTexture
//       Access: Published
//  Description: Destructor. Release the camera or video stream
////////////////////////////////////////////////////////////////////
AviTexture::~AviTexture()
{
     cvReleaseCapture(&_capture);
}

////////////////////////////////////////////////////////////////////
//     Function: AviTexture::gen_tex
//       Access: Published
//  Description: Tries to find the largest texture that will fit
//               inside the video stream. TODO: allow for fit around
////////////////////////////////////////////////////////////////////
void AviTexture::gen_tex(int magicNum)
{
  if(magicNum<32)
  {
    magicNum=16;
  }
  else if(magicNum<64)
  {
    magicNum=32;
  }
  else if(magicNum<128)
  {
    magicNum=64;
  }
  else if(magicNum<256)
  {
    magicNum=128;
  }
  else if(magicNum<512)
  {
    magicNum=256;
  }
  else if(magicNum<1024)
  {
    magicNum=512;
  }
  else
    magicNum=1024;
  setup_2d_texture(magicNum,magicNum, Texture::T_unsigned_byte,Texture::F_rgb8);
  _magicNum=magicNum;
}

////////////////////////////////////////////////////////////////////
//     Function: AviTexture::update
//       Access: Published
//  Description: Grabs the next frame off the camera or avi file
//               Returns false if the capture fails or reached EOF 
////////////////////////////////////////////////////////////////////

bool AviTexture::update()
{
  int begx,endx,begy,endy;
  IplImage* frame = 0;
  int xs,ys;
  if(_capture)
  {
    if(_time==1)
      return false;
    frame = cvQueryFrame( _capture );
    if(frame)
    {
      _time=cvGetCaptureProperty(_capture,CV_CAP_PROP_POS_AVI_RATIO);
      _current_frame=cvGetCaptureProperty(_capture,CV_CAP_PROP_POS_FRAMES);
      if(!obtain_ram())
	return false;
      begx=(_width-_magicNum)/2.0;
      endx=_width-begx;
      begy=(_height-_magicNum)/2.0;
      endy=_height-begy;
      
      if(_buf)
      {
	
	xs=get_x_size();
	ys=get_y_size();
	
	if(get_num_components()!=3)
	  return false;
	
	for(int i=begy;i<endy;i++)
	  memcpy(_buf+((i-begy)*xs*3),frame->imageData+i*+_width*3+begx*3,xs*3);
	
	return true;
      }
    }
  }
  return false;
}

#endif
