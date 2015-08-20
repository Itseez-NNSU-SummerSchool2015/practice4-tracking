#include <tracker.hpp>

class TrackerShchedrin : public Tracker
{
 public:
    virtual ~TrackerShchedrin() {}

    virtual bool init( const cv::Mat& frame, const cv::Rect& initial_position );
    virtual bool track( const cv::Mat& frame, cv::Rect& new_position );

 private:
    cv::Rect position_;
};

bool TrackerShchedrin::init( const cv::Mat& frame, const cv::Rect& initial_position )
{
    position_ = initial_position;
	return true;
}

bool TrackerShchedrin::track( const cv::Mat& frame, cv::Rect& new_position )
{
    new_position = position_;
	return true;
}

cv::Ptr<Tracker> createTrackerShchedrin()
{
    return cv::Ptr<Tracker>(new TrackerShchedrin());
}
