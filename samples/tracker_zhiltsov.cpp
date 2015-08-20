#include <tracker.hpp>
#include <vector>
#include <deque>
#include "opencv2\core\core.hpp"
#include "opencv2\imgproc\imgproc.hpp"
#include "opencv2\highgui\highgui.hpp"
#include "opencv2\video\tracking.hpp"
#include <iostream>

class TrackerZhiltsov : public Tracker
{
public:
    virtual bool init( const cv::Mat& frame, const cv::Rect& initial_position );
    virtual bool track( const cv::Mat& frame, cv::Rect& new_position );

private:
    static const uchar MAX_POINTS;
    static const double DEFAULT_QUALITY;
    static const double MIN_POINTS_DISTANCE;
    static const size_t MAX_FRAMES;

    typedef cv::Point2f Point;
    typedef std::vector<Point> Points;
    typedef cv::Mat Frame;
    typedef std::deque<Frame> Frames;

    cv::Rect position;
    Points control_points;
    Frames frames_cache;   

    void prepareFrame(const cv::Mat& src, cv::Mat& dst);
    bool acceptFrame(const Frame& frame);
    void selectNewPoints(Points& prev_points, Points& new_points, const std::vector<uchar>& status, const std::vector<float>& error);
    float findMedian(const std::vector<float>& vec);
};

const uchar TrackerZhiltsov::MAX_POINTS = 100;
const double TrackerZhiltsov::DEFAULT_QUALITY = 0.2;
const double TrackerZhiltsov::MIN_POINTS_DISTANCE = 5.0;
const size_t TrackerZhiltsov::MAX_FRAMES = 20;


void TrackerZhiltsov::prepareFrame( const cv::Mat& src, cv::Mat& dst ) 
{
    cvtColor(src, dst, CV_BGR2GRAY);
}

bool TrackerZhiltsov::acceptFrame( const Frame& frame ) 
{
    bool removed = false;
    if (MAX_FRAMES < frames_cache.size() + 1) 
    {
        frames_cache.pop_front();
        removed = true;
    }
    frames_cache.push_back(frame.clone());
    return removed;
}

bool TrackerZhiltsov::init( const cv::Mat& frame, const cv::Rect& initial_position )
{
    cv::Mat monochrome;
    prepareFrame(frame, monochrome);
    acceptFrame(monochrome);

    Frame roiMask = cv::Mat::zeros(cv::Size(monochrome.cols, monochrome.rows), monochrome.type());
    cv::rectangle(roiMask, initial_position, CV_RGB(255, 255, 255), CV_FILLED);
    cv::goodFeaturesToTrack(monochrome, control_points, MAX_POINTS, DEFAULT_QUALITY, MIN_POINTS_DISTANCE, roiMask);

    for (auto it = control_points.cbegin(); it != control_points.cend(); ++it) {
        cv::circle(monochrome, *it, 3, CV_RGB(255, 0, 0));
    }

    cv::imshow("Mono", monochrome);
    
    position = cv::boundingRect(control_points);

	return true;
}

bool TrackerZhiltsov::track( const cv::Mat& frame, cv::Rect& new_position )
{
    cv::Mat monochrome;
    prepareFrame(frame, monochrome);

    CV_Assert(!frames_cache.back().empty());
    CV_Assert(!monochrome.empty());

    Points new_points;
    std::vector<uchar> flow_found;
    std::vector<float> error;
    cv::calcOpticalFlowPyrLK(
        frames_cache.back(), monochrome, 
        control_points, new_points,
        flow_found, error
        );

    if (new_points.size() < 2) {
        return false;
    }

    selectNewPoints(control_points, new_points, flow_found, error);
    
    if (control_points.size() < 2) {
        std::swap(control_points, new_points);
        return false;
    }

    position = cv::boundingRect(control_points);
    new_position = position;

    Frame roiMask = cv::Mat::zeros(cv::Size(monochrome.cols, monochrome.rows), monochrome.type());
    cv::rectangle(roiMask, position, CV_RGB(255, 255, 255), CV_FILLED);
    cv::goodFeaturesToTrack(monochrome, new_points, MAX_POINTS, DEFAULT_QUALITY, MIN_POINTS_DISTANCE, roiMask);
    control_points.insert(control_points.begin(), new_points.cbegin(), new_points.cend());    

    for (auto it = control_points.cbegin(); it != control_points.cend(); ++it) {
        cv::circle(monochrome, *it, 3, CV_RGB(255, 0, 0));
    }

    cv::imshow("Mono", monochrome);
//    cv::imshow("Mask", roiMask);

    acceptFrame(monochrome);
	return true;
}

float TrackerZhiltsov::findMedian(const std::vector<float>& vec)
{
    auto sequence = vec;
    auto it = sequence.begin() + sequence.size() / 2;
    std::nth_element(sequence.begin(), it, sequence.end());
    return *it;
}

void TrackerZhiltsov::selectNewPoints(Points& prev_points, Points& new_points, const std::vector<uchar>& status, const std::vector<float>& error) 
{
    const float medianError = findMedian(error) + 1.f;
    
    prev_points.clear();
    for (auto it = new_points.cbegin(), iend = new_points.cend(); it != iend; ++it) {
        const auto pos = it - new_points.cbegin();
        if ((status[pos] == 1) && (error[pos] < medianError))
        {
            prev_points.push_back(*it);
        }
    }
}

cv::Ptr<Tracker> createTrackerZhiltsov()
{
    return cv::Ptr<Tracker>(new TrackerZhiltsov());
}
