#include <tracker.hpp>
#include <vector>
#include <ctype.h>
#include <iostream>

#include "opencv2/video/tracking.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"

using namespace std;
using namespace cv;


void featuresStatus(vector<unsigned char>&, vector<Point2f>&, vector<Point2f>&, vector<float>&, float);
float median(vector<float>);
void ComputeShift(Rect&, const Mat&, const vector<Point2f>&, const vector<Point2f>);


class TrackerAwesome: public Tracker
{
public:
    virtual ~TrackerAwesome() {}

    virtual bool init( const cv::Mat& frame, const cv::Rect& initial_position );
    virtual bool track( const cv::Mat& frame, cv::Rect& new_position );

private:
    cv::Mat prev_gray;
    cv::Rect position_;
};

bool TrackerAwesome::init( const cv::Mat& frame, const cv::Rect& initial_position )
{
    position_ = initial_position;
    cvtColor(frame, prev_gray, COLOR_BGR2GRAY);
    return true;
}

bool TrackerAwesome::track( const cv::Mat& frame, cv::Rect& new_position )
{
    new_position = position_;

    Mat roi(prev_gray, position_);
    vector<Point2f> features, features_new;
    goodFeaturesToTrack(roi, features, 50, 0.01, 10);
    for (int i = 0; i < features.size(); i++)
    {
        features[i].x += position_.x;
        features[i].y += position_.y;
    }

    // Optical flow
    vector<unsigned char> status;
    vector<float> errors;
    Mat gray;
    cvtColor(frame, gray, COLOR_BGR2GRAY);
    calcOpticalFlowPyrLK(prev_gray, gray, features, features_new, status, errors);

    /*
    for (int i = 0; i < features_new.size(); i++)
    {
        cv::circle(gray, features_new[i], 5, Scalar(0, 0, 255));
    }
    imshow("feature points", gray);
    cout << features.size() << " " << features_new.size() << endl;
    
    cout << endl;
    
    waitKey();
    */

    float medianError = median(errors); 
    featuresStatus(status, features, features_new, errors, medianError);  

    // Shift rectangle on axis 
    ComputeShift(new_position, frame, features, features_new);

    for (int i = 0; i < features_new.size(); i++)
    {
        cv::circle(gray, features_new[i], 5, Scalar(0, 0, 255));
    }
    imshow("2", gray);

    // TODO: coefficient scale + median shift;

    cv::swap(prev_gray, gray);
    position_ = new_position;

    return true;
}

cv::Ptr<Tracker> createTrackerAwesome()
{
    return cv::Ptr<Tracker>(new TrackerAwesome());
}

void featuresStatus(vector<unsigned char>& status, vector<Point2f>& features, vector<Point2f>& features_new, vector<float>& errors, float median)
{
    for (int i = status.size() - 1; i >= 0; i--)
    {
        if (!status[i])
        {
            status.erase(status.begin() + i);
            features.erase(features.begin() + i);

            errors.erase(errors.begin() + i);
        }
    }
}

float median(vector<float> errors)
{
    sort(errors.begin(), errors.end());
    return errors[errors.size() / 2];
}

void ComputeShift(Rect& new_position, const Mat& frame, const vector<Point2f>& features, const vector<Point2f> features_new)
{
    float shiftX, shiftY;

    std::vector<float> x(features.size());
    std::vector<float> y(features.size());

    for (int i = 0; i < features.size(); i++)
    {
        x[i] = features_new[i].x - features[i].x;
        y[i] = features_new[i].y - features[i].y;
    }

    std::sort(x.begin(), x.end());
    std::sort(y.begin(), y.end());

    shiftX = x[x.size() / 2];
    shiftY = y[y.size() / 2];

    // X+
    if (new_position.x + shiftX + new_position.width > frame.cols)
    {
        new_position.x = frame.cols - new_position.width;
    }
    // X-
    else if (new_position.x + shiftX < 0)
    {
        new_position.x = 0;
    }
    else
    {
        new_position.x += shiftX;
    }

    // Y+
    if (new_position.y + shiftY + new_position.height > frame.rows)
    {
        new_position.y = frame.rows - new_position.height;
    }
    // Y-
    else if (new_position.y + shiftY < 0)
    {
        new_position.y = 0;
    }
    else
    {
        new_position.y += shiftY;
    }
}