#include <tracker.hpp>

class TrackerDummy : public Tracker
{
 public:
    virtual ~TrackerDummy() {}

    virtual bool init( const cv::Mat& frame, const cv::Rect& initial_position );
    virtual bool track( const cv::Mat& frame, cv::Rect& new_position );

 private:
    cv::Rect position_;
};

bool TrackerDummy::init( const cv::Mat& frame, const cv::Rect& initial_position )
{
    position_ = initial_position;
	return true;
}

bool TrackerDummy::track( const cv::Mat& frame, cv::Rect& new_position )
{
    new_position = position_;
	return true;
}

cv::Ptr<Tracker> createTrackerDummy()
{
    return cv::Ptr<Tracker>(new TrackerDummy());
}
