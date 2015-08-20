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
    static const int MAX_POINT_COUNT = 250;
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
  if(scores.size() == 0){
      return 0;
  }
  if(scores.size() == 1){
      return scores[0];
  }
  if(scores.size() == 2){
      return scores[0]/2 + scores[1]/2;
  }
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
    std::vector<unsigned char> status0, status_fb, status_merge, status_final;
    std::vector<cv::Point2f> points, new_points, ok1points, points_fb;
    std::vector<float> err0, err_fb;
    std::vector<double> shift_vecx, shift_vecy;
    cv::goodFeaturesToTrack(prevFrameGray(position_), points, MAX_POINT_COUNT, 0.01, 5);
    cv::Mat debugShow = frame.clone();
    for(int i = 0; i < points.size(); i++){
        points[i].x += position_.x;
        points[i].y += position_.y;
        circle(debugShow,points[i],2,cv::Scalar(0,0,0),2);
    }
    if(points.size() == 0){
        return false;
    }
    cv::calcOpticalFlowPyrLK(prevFrameGray, gray, points, new_points, status0, err0);
    cv::calcOpticalFlowPyrLK(gray, prevFrameGray, new_points, points_fb, status_fb, err_fb);
    status_merge.resize(status0.size());
    status_final.resize(status0.size());
    for(int i = 0; i < new_points.size(); i++){
        status_merge[i] = status0[i] && status_fb[i]; 
    }

    double errlimit = 0;
    double shiftX = 0, shiftY = 0;
    double distLim;
    std::vector<double> distances;
    std::vector<double> err_vec;

    for(int i = 0; i < points.size(); i++){
        if(status_merge[i]){
            distances.push_back(cv::norm(points[i] - points_fb[i]));
        }
    }
    distLim = CalcMedian(distances);

    for(int i = 0; i < points.size(); i++){
        if(status_merge[i]){
            if(cv::norm(points[i] - points_fb[i]) > distLim){
                status_final[i] = 0;
            }
            else{
                status_final[i] = 1;
            }
        }
    }

    for(int i = 0; i < points.size(); i++){
        if(status_final[i]){
            err_vec.push_back(err0[i]);
        }
    }
    errlimit = CalcMedian(err_vec);
    for(int i = 0; i < points.size(); i++){
        if(status_final[i]){
            if(err0[i] < errlimit){
                shift_vecx.push_back((new_points[i] - points[i]).x);
                shift_vecy.push_back((new_points[i] - points[i]).y);
            }
        }
    }
    std::vector<double> scalesX,scalesY;
    double zoomX, zoomY;
    for(int i = 0; i < points.size(); i++){
        for(int j = 0; j < points.size(); j++){
            if(status_final[i] && status_final[j] && i != j){
                if(err0[i] < errlimit){
                    double scaleX = cv::norm(new_points[i].x - new_points[j].x)/cv::norm(points[i].x - points[j].x);
                    double scaleY = cv::norm(new_points[i].y - new_points[j].y)/cv::norm(points[i].y - points[j].y);
                    scaleX = abs(scaleX);
                    scaleY = abs(scaleY);
                    scalesX.push_back(scaleX);
                    scalesY.push_back(scaleY);
                }
            }
        }
    }
    zoomX = CalcMedian(scalesX);
    zoomY = CalcMedian(scalesY);

    if(shift_vecx.size() >= 2){
        shiftX = CalcMedian(shift_vecx);
        shiftY = CalcMedian(shift_vecy);
    }
    new_position = position_ + cv::Point(shiftX,shiftY);
    new_position.width *= zoomX;
    new_position.height *= zoomY;
    if(new_position.width < 5){
        new_position.width = 5;
    }
    if(new_position.height < 5){
        new_position.height = 5;
    }
    if(new_position.x < 0){
        new_position.x = 0;
    }
    if(new_position.y < 0){
        new_position.y = 0;
    }
    if(new_position.x > frame.cols - new_position.width ){
        new_position.x = frame.cols - new_position.width;
    }
    if(new_position.y > frame.rows - new_position.height ){
        new_position.y = frame.rows - new_position.height;
    }
    position_ = new_position;
    prevFrameGray = gray.clone();

    for(int i = 0; i < points.size(); i++){
        if(status_final[i]){
            circle(debugShow,points[i],2,cv::Scalar(0,200,0),2);
        }
    }
    cv::Mat flipped;
    cv::flip(debugShow,flipped,1);
    cv::imshow("debug",flipped);
	return true;
}

cv::Ptr<Tracker> createTrackerShchedrin()
{
    return cv::Ptr<Tracker>(new TrackerShchedrin());
}
