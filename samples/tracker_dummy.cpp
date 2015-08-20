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

class TrackerIlya : public Tracker
{
 public:
    virtual ~TrackerIlya() {}

    virtual bool init( const cv::Mat& frame, const cv::Rect& initial_position );
    virtual bool track( const cv::Mat& frame, cv::Rect& new_position );

 private:
    cv::Rect position_;
};

bool TrackerIlya::init( const cv::Mat& frame, const cv::Rect& initial_position )
{
    position_ = initial_position;
	return true;
}

bool TrackerIlya::track( const cv::Mat& frame, cv::Rect& new_position )
{
    long x=position_.x;
    long y=position_.y;
    long width=position_.width;
    long height=position_.height;
	x++;y++;
    {
    cv::Rect t(x,y,width,height);
	position_=t;
    }
    new_position = position_;
	return true;
}

cv::Ptr<Tracker> createTrackerIlya()
{
    return cv::Ptr<Tracker>(new TrackerIlya());
}
