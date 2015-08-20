#include <iostream>
#include <string>
#include <vector>
#include <iostream>

#include <tracker.hpp>

#include <opencv2/video/tracking.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include "opencv2/core/core.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/objdetect/objdetect.hpp"

using namespace cv;
using namespace std;

class TrackerMoskalenko : public Tracker
{
public:
    cv::Mat image_prev, image_next;
    std::vector<cv::Point2f> features_prev, features_next;
    std::vector<uchar> status;
    std::vector<float> err;
    std::vector<uchar> status_back;
    std::vector<float> err_back;
public:
   virtual ~TrackerMoskalenko() 
   {
   }

   virtual bool init( const cv::Mat& frame, const cv::Rect& initial_position );
   virtual bool track( const cv::Mat& frame, cv::Rect& new_position );

private:
   cv::Rect position_;
};

bool TrackerMoskalenko::init( const cv::Mat& frame, const cv::Rect& initial_position )
{
    Mat image_gray;
    cv::cvtColor(frame,image_gray,CV_BGR2GRAY);
    image_next = image_gray.clone();
    cv::goodFeaturesToTrack(image_next(initial_position), 
        features_next,
        1000, 0.5, 7);
    position_ = initial_position;
    return true;
}

bool TrackerMoskalenko::track( const cv::Mat& frame, cv::Rect& new_position )
{
    image_prev = image_next.clone();
    features_prev = features_next;

    Mat image_gray;
    cv::cvtColor(frame,image_gray,CV_BGR2GRAY);
    image_next = image_gray.clone();

    cv::calcOpticalFlowPyrLK(
      image_prev, image_next,
      features_prev,
      features_next,
      status,
      err
    );

    Mat a = frame.clone();
    for (int i = 0 ; i < features_next.size(); i++)
    {
        cv::circle(a,features_next[i],2,Scalar(0,255,0));
    }
    imshow("features_next",a);
    waitKey(20);

    vector<float> x,y,err_copy;
    err_copy = err;
    sort(err_copy.begin(),err_copy.end());
    float p = err_copy[err_copy.size()/2];
    vector<Point2f> res;


    for (int i = 0 ; i < features_next.size(); i++)
    {
        if (err[i]<p)
        {
            x.push_back(features_next[i].x - features_prev[i].x);
            y.push_back(features_next[i].y - features_prev[i].y);
        }
    }


    if (x.size() > 0)
    {
        sort(x.begin(),x.end());
        sort(y.begin(),y.end());
        position_.x += x[x.size()/2];
        position_.y += y[y.size()/2];

        new_position = position_;
    }
    return true;
}

cv::Ptr<Tracker> createTrackerMoskalenko()
{
   return cv::Ptr<Tracker>(new TrackerMoskalenko());
}