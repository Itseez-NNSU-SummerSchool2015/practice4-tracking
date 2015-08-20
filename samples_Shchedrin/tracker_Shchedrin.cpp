#include <tracker.hpp>
#include <iostream>
#include "opencv2/video/tracking.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"

class TrackerShchedrin : public Tracker
{
 public:
    virtual ~TrackerShchedrin() {}

    virtual bool init( const cv::Mat& frame, const cv::Rect& initial_position );
    virtual bool track( const cv::Mat& frame, cv::Rect& new_position );

 private:
    static const int MAX_POINT_COUNT = 50;
    cv::Mat prevFrameGray;
    cv::Rect position_;
};

bool TrackerShchedrin::init( const cv::Mat& frame, const cv::Rect& initial_position )
{
    position_ = initial_position;
    cv::cvtColor(frame, prevFrameGray,cv::COLOR_BGR2GRAY);
	return true;
}

double CalcMedian(std::vector<double> scores)
{
  double median;
  size_t size = scores.size();
  sort(scores.begin(), scores.end());
  if (size  % 2 == 0)
  {
      median = (scores[size / 2 - 1] + scores[size / 2]) / 2;
  }
  else 
  {
      median = scores[size / 2];
  }
  return median;
}

bool TrackerShchedrin::track( const cv::Mat& frame, cv::Rect& new_position )
{
    cv::Mat gray;
    cv::cvtColor(frame, gray,cv::COLOR_BGR2GRAY);
    std::vector<unsigned char> status;
    std::vector<cv::Point2f> points, new_points;
    std::vector<float> err;
    std::vector<double> shift_vecx, shift_vecy;

    cv::goodFeaturesToTrack(prevFrameGray, points, MAX_POINT_COUNT, 0.01, 10);
    cv::calcOpticalFlowPyrLK(prevFrameGray, gray, points, new_points, status, err);


    double totalweight = 0;
    double errlimit = 0;
    double shiftX = 0, shiftY = 0;
    std::vector<double> err_vec;
    for(int i = 0; i < points.size(); i++){
        if(status[i]){
            err_vec.push_back(err[i]);
        }
    }
    errlimit = CalcMedian(err_vec);
    for(int i = 0; i < points.size(); i++){
        if(status[i]){
            if(err[i] < errlimit){
                shift_vecx.push_back((new_points[i] - points[i]).x);
                shift_vecy.push_back((new_points[i] - points[i]).y);
            }
        }
    }
    std::cout<<errlimit<<std::endl;
    shiftX = CalcMedian(shift_vecx);
    shiftY = CalcMedian(shift_vecy);
    new_position = position_ + cv::Point(shiftX,shiftY);
    position_ = new_position;
    prevFrameGray = gray.clone();
	return true;
}

cv::Ptr<Tracker> createTrackerShchedrin()
{
    return cv::Ptr<Tracker>(new TrackerShchedrin());
}
