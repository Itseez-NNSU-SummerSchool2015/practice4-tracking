#include <tracker.hpp>
#include <opencv2/video/tracking.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <iostream>

class TrackerKustikova : public Tracker
{
 public:
    virtual ~TrackerKustikova() {}

    virtual bool init( const cv::Mat& frame, const cv::Rect& initial_position );
    virtual bool track( const cv::Mat& frame, cv::Rect& new_position );

 private:
    cv::Rect position_;
    cv::Mat prevFrame_;

    bool filterCorners(std::vector<cv::Point2f> &corners, 
        std::vector<cv::Point2f> &nextCorners, std::vector<uchar> &status,
        std::vector<float> &errors);

    bool restoreBoundingBox(std::vector<cv::Point2f> &corners, 
        std::vector<cv::Point2f> &nextCorners, cv::Rect &new_position);

    bool computeMedianShift(std::vector<cv::Point2f> &corners, 
        std::vector<cv::Point2f> &nextCorners, float &dx, float &dy);

    bool computeScaleFactor(std::vector<cv::Point2f> &corners, 
        std::vector<cv::Point2f> &nextCorners, float &scale);

    bool computePointDistances(std::vector<cv::Point2f> &corners,
        std::vector<float> &dist);

    bool computeDistScales(std::vector<float> &dist, 
        std::vector<float> &nextDist, std::vector<float> &distScales);
};

bool TrackerKustikova::init( const cv::Mat& frame, const cv::Rect& initial_position )
{
    position_ = initial_position;
    cv::cvtColor(frame, prevFrame_, CV_BGR2GRAY);
	return true;
}

bool TrackerKustikova::filterCorners(std::vector<cv::Point2f> &corners, 
        std::vector<cv::Point2f> &nextCorners, std::vector<uchar> &status,
        std::vector<float> &errors)
{
    std::cout << corners.size() << " " << nextCorners.size() << " " <<
        status.size() << " " << errors.size() << std::endl;
    for (int i = status.size() - 1; i >= 0; i--)
    {
        if (!status[i])
        {
            status.erase(status.begin() + i);
            corners.erase(corners.begin() + i);
            errors.erase(errors.begin() + i);
        }
    }
    if (corners.empty())
    {
        return false;
    }
    std::vector<float> errorsCopy(errors.size());
    std::copy(errors.begin(), errors.end(), errorsCopy.begin());
    std::sort(errorsCopy.begin(), errorsCopy.end());
    float medianError = errorsCopy[errorsCopy.size() / 2];

    for (int i = errors.size() - 1; i >= 0; i--)
    {
        if (errors[i] > medianError)
        {
            errors.erase(errors.begin() + i);
            corners.erase(corners.begin() + i);
            nextCorners.erase(nextCorners.begin() + i);
            status.erase(status.begin() + i);
        }
    }
    if (corners.empty())
    {
        return false;
    }

    return true;
}

bool TrackerKustikova::computeMedianShift(std::vector<cv::Point2f> &corners, 
        std::vector<cv::Point2f> &nextCorners, float &dx, float &dy)
{
    std::vector<float> shiftOx, shiftOy;
    for (int i = 0; i < corners.size(); i++)
    {
        shiftOx.push_back(nextCorners[i].x - corners[i].x);
        shiftOy.push_back(nextCorners[i].y - corners[i].y);
    }
    std::sort(shiftOx.begin(), shiftOx.end());
    std::sort(shiftOy.begin(), shiftOy.end());
    dx = shiftOx[shiftOx.size() / 2];
    dy = shiftOy[shiftOy.size() / 2];
    return true;
}

bool TrackerKustikova::computePointDistances(std::vector<cv::Point2f> &corners,
        std::vector<float> &dist)
{
    dist.clear();
    for (int i = 0; i < corners.size(); i++)
    {
        for (int j = i + 1; j < corners.size(); j++)
        {
            dist.push_back(cv::norm(corners[i] - corners[j]));
        }
    }
    return true;
}

bool TrackerKustikova::computeDistScales(std::vector<float> &dist, 
        std::vector<float> &nextDist, std::vector<float> &distScales)
{
    distScales.clear();
    for (int i = 0; i < dist.size(); i++)
    {
        distScales.push_back(nextDist[i] / dist[i]);
    }
    return true;
}

bool TrackerKustikova::computeScaleFactor(std::vector<cv::Point2f> &corners,
        std::vector<cv::Point2f> &nextCorners, float &scale)
{
    if (corners.size() <= 1 || nextCorners.size() <= 1)
    {
        return false;
    }
    std::vector<float> dist, nextDist, distScales;
    computePointDistances(corners, dist);
    computePointDistances(nextCorners, nextDist);
    computeDistScales(dist, nextDist, distScales);
    std::sort(distScales.begin(), distScales.end());
    scale = distScales[distScales.size() / 2];
    return true;
}

bool TrackerKustikova::restoreBoundingBox(std::vector<cv::Point2f> &corners,
        std::vector<cv::Point2f> &nextCorners, cv::Rect &new_position)
{
    float dx, dy;
    computeMedianShift(corners, nextCorners, dx, dy);
    float ddx = position_.x + dx, ddy = position_.y + dy;
    if (ddx >= prevFrame_.size().width || ddy >= prevFrame_.size().height)
    {
        return false;
    }
    new_position.x = (ddx < 0) ? 0 : ddx;
    new_position.y = (ddy < 0) ? 0 : ddy;

    float scale;
    if (!computeScaleFactor(corners, nextCorners, scale))
    {
        return false;
    }
    float width, height;
    width = position_.width * scale;
    height = position_.height * scale;

    float x = ddx + width, y = ddy + height;
    x = (x >= prevFrame_.size().width) ? prevFrame_.size().width - 1 : x;
    y = (y >= prevFrame_.size().height) ? prevFrame_.size().height - 1 : y;
    new_position.width = x - new_position.x;
    new_position.height = y - new_position.y;

    return true;
}

bool TrackerKustikova::track( const cv::Mat& frame, cv::Rect& new_position )
{
    cv::Mat object = prevFrame_(position_);
    std::vector<cv::Point2f> corners;
    const int maxCorners = 100;
    cv::goodFeaturesToTrack(object, corners, maxCorners, 0.01, 5);
    if (corners.empty())
    {
        std::cout << "Tracked object is lost." << std::endl;
        return false;
    }
    for (int i = 0; i < corners.size(); i++)
    {
        corners[i].x += position_.x;
        corners[i].y += position_.y;
    }

    std::vector<cv::Point2f> nextCorners;
    std::vector<uchar> status;
    std::vector<float> errors;
    cv::Mat frame_;
    cv::cvtColor(frame, frame_, CV_BGR2GRAY);
    cv::calcOpticalFlowPyrLK(prevFrame_, frame_, 
        corners, nextCorners, status, errors);

    if (!filterCorners(corners, nextCorners, status, errors))
    {
        std::cout << "There no feature points for tracking." << std::endl;
        return false;
    }

    if (!restoreBoundingBox(corners, nextCorners, new_position))
    {
        std::cout << "There no enough number of feature points " <<
            "to restore bounding box." << std::endl;
        return false;
    }

    position_ = new_position;
    prevFrame_ = frame_;
	return true;
}

cv::Ptr<Tracker> createTrackerKustikova()
{
    return cv::Ptr<Tracker>(new TrackerKustikova());
}
