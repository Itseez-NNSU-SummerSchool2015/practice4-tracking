#include <tracker.hpp>
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

bool TrackerShchedrin::track( const cv::Mat& frame, cv::Rect& new_position )
{
    cv::Mat gray;
    cv::cvtColor(frame, gray,cv::COLOR_BGR2GRAY);
    std::vector<unsigned char> status;
    std::vector<cv::Point2f> points, new_points;
    std::vector<float> err;

    cv::goodFeaturesToTrack(prevFrameGray, points, MAX_POINT_COUNT, 0.01, 10);
    cv::calcOpticalFlowPyrLK(prevFrameGray, gray, points, new_points, status, err);


    new_position = position_;
    prevFrameGray = gray.clone();
	return true;
}

cv::Ptr<Tracker> createTrackerShchedrin()
{
    return cv::Ptr<Tracker>(new TrackerShchedrin());
}
