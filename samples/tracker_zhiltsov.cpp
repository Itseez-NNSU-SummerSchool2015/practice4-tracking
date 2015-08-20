#include <tracker.hpp>

class TrackerZhiltsov : public Tracker
{
 public:
    virtual bool init( const cv::Mat& frame, const cv::Rect& initial_position );
    virtual bool track( const cv::Mat& frame, cv::Rect& new_position );

 private:
    cv::Rect position_;
};

bool TrackerZhiltsov::init( const cv::Mat& frame, const cv::Rect& initial_position )
{
    position_ = initial_position;
	return true;
}

bool TrackerZhiltsov::track( const cv::Mat& frame, cv::Rect& new_position )
{
    new_position = position_;
	return true;
}

cv::Ptr<Tracker> createTrackerZhiltsov()
{
    return cv::Ptr<Tracker>(new TrackerZhiltsov());
}
