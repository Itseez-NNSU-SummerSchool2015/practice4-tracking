#include <tracker.hpp>

class TrackerKustikova : public Tracker
{
 public:
    virtual ~TrackerKustikova() {}

    virtual bool init( const cv::Mat& frame, const cv::Rect& initial_position );
    virtual bool track( const cv::Mat& frame, cv::Rect& new_position );

 private:
    cv::Rect position_;
};

bool TrackerKustikova::init( const cv::Mat& frame, const cv::Rect& initial_position )
{
    position_ = initial_position;
	return true;
}

bool TrackerKustikova::track( const cv::Mat& frame, cv::Rect& new_position )
{
    new_position = position_;
	return true;
}

cv::Ptr<Tracker> createTrackerKustikova()
{
    return cv::Ptr<Tracker>(new TrackerKustikova());
}
