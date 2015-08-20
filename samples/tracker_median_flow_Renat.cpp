#include <tracker.hpp>
#include <opencv2\core\core.hpp>
#include <opencv2\imgproc\imgproc.hpp>
#include <opencv2\highgui\highgui.hpp>
#include <opencv2\video\tracking.hpp>
#include <vector>
#include <algorithm>
#include <iostream>

class TrackerRenat : public Tracker
{
public:
   virtual ~TrackerRenat() {}

   virtual bool init( const cv::Mat& frame, const cv::Rect& initial_position );
   virtual bool track( const cv::Mat& frame, cv::Rect& new_position );

private:
   cv::Mat oldFrame;
   cv::Rect position_;
};

bool TrackerRenat::init( const cv::Mat& frame, const cv::Rect& initial_position )
{
   frame.copyTo(oldFrame);
   position_ = initial_position;
   return true;
}

bool TrackerRenat::track( const cv::Mat& frame, cv::Rect& new_position )
{
   std::vector<cv::Point2f>  cor;
   cv::Mat gray, prevGray, gray4cor, show;

   cv::cvtColor(oldFrame(position_), gray4cor, cv::COLOR_BGR2GRAY);
   
   cv::goodFeaturesToTrack(gray4cor, cor, 100, 0.1, 4);

   frame.copyTo(show);
   for(int i = 0; i < cor.size(); i++)
   {
       cor[i].x += position_.x;
       cor[i].y += position_.y;
       cv::circle(show, cor[i], 2, cv::Scalar(0,0,0));
   }
   


   std::vector<uchar> status;
   std::vector<float> err;
   std::vector<cv::Point2f>  newCor;

   cv::cvtColor(frame, gray, cv::COLOR_BGR2GRAY);
   cv::cvtColor(oldFrame, prevGray, cv::COLOR_BGR2GRAY);

   cv::calcOpticalFlowPyrLK(prevGray, gray, cor, newCor, status, err);

   for(int i = 0; i < cor.size(); i++)
   {
       cv::circle(show, newCor[i], 2, cv::Scalar(0,0,0));
   }
   cv::imshow("blablabla1", show);
   cv::waitKey(1);




   frame.copyTo(oldFrame);

   std::vector<float>  deltax;
   std::vector<float>  deltay;

   for(int i = 0; i < cor.size(); i++)
   {
       if(status[i] == 1)
       {
           deltax.push_back(newCor[i].x - cor[i].x);
           deltay.push_back(newCor[i].y - cor[i].y);
       }
   }

   std::sort(deltax.begin(), deltax.end());
   std::sort(deltay.begin(), deltay.end());

   int dx = deltax[deltax.size() / 2]; 
   int dy = deltay[deltay.size() / 2];


   position_.x += dx;
   position_.y += dy;

   new_position = position_;

   /*std::vector<cv::Point2f>  rCor, gCor, bCor;
   cv::Mat red, prevRed, red4cor;
   std::vector<cv::Mat> rgb(3);
   std::vector<cv::Mat> rgbn(3);
   cv::split(oldFrame, rgb);
   cv::split(oldFrame, rgbn);

   cv::goodFeaturesToTrack(rgb[0], bCor, 100, 0.1, 1);
   cv::goodFeaturesToTrack(rgb[1], gCor, 100, 0.1, 1);
   cv::goodFeaturesToTrack(rgb[2], rCor, 100, 0.1, 1);


   std::vector<uchar> rstatus, bstatus, gstatus;
   std::vector<float> rerr, berr, gerr;
   std::vector<cv::Point2f>  rnewCor, gnewCor, bnewCor;


   cv::calcOpticalFlowPyrLK(rgb[0], rgbn[0], bCor, bnewCor, bstatus, berr);
   cv::calcOpticalFlowPyrLK(rgb[1], rgbn[1], gCor, gnewCor, gstatus, gerr);
   cv::calcOpticalFlowPyrLK(rgb[2], rgbn[2], rCor, rnewCor, rstatus, rerr);

   frame.copyTo(oldFrame);

   std::vector<float>  rdeltax, bdeltax, gdeltax;
   std::vector<float>  rdeltay, bdeltay, gdeltay;

   for(int i = 0; i < bCor.size(); i++)
   {
       if(bstatus[i] == 1)
       {
           bdeltax.push_back(bnewCor[i].x - bCor[i].x);
           bdeltay.push_back(bnewCor[i].y - bCor[i].y);
       }
   }
   for(int i = 0; i < rCor.size(); i++)
   {
       if(rstatus[i] == 1)
       {
           rdeltax.push_back(rCor[i].x - rCor[i].x);
           rdeltay.push_back(rCor[i].y - rCor[i].y);
       }
   }
   for(int i = 0; i < gCor.size(); i++)
   {
       if(gstatus[i] == 1)
       {
           gdeltax.push_back(bnewCor[i].x - gCor[i].x);
           gdeltay.push_back(bnewCor[i].y - gCor[i].y);
       }
   }


   std::sort(bdeltax.begin(), bdeltax.end());
   std::sort(bdeltay.begin(), bdeltay.end());

   int bdx = bdeltax[bdeltax.size() / 2]; 
   int bdy = bdeltay[bdeltay.size() / 2];

   std::sort(rdeltax.begin(), rdeltax.end());
   std::sort(rdeltay.begin(), rdeltay.end());

   int rdx = rdeltax[rdeltax.size() / 2]; 
   int rdy = rdeltay[rdeltay.size() / 2];

   std::sort(gdeltax.begin(), gdeltax.end());
   std::sort(gdeltay.begin(), gdeltay.end());

   int gdx = gdeltax[gdeltax.size() / 2]; 
   int gdy = gdeltay[gdeltay.size() / 2];


   int dx = (bdx + gdx + rdx) / 3;
   int dy = (bdy + gdy + rdy) / 3;

   position_.x += dx;
   position_.y += dy;

   new_position = position_;*/

   return true;
}

cv::Ptr<Tracker> createTrackerRenat()
{
   return cv::Ptr<Tracker>(new TrackerRenat());
}