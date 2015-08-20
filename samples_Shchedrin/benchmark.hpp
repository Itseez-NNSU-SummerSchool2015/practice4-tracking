#pragma once

#include <fstream>
#include <iostream>
#include <cmath>
#include <opencv2/core/core.hpp>

inline float overlap(const cv::Rect &guess, const cv::Rect &gt)
{
    if (guess == cv::Rect() && gt == cv::Rect())
        return 1.0f;

    cv::Rect intersection = guess & gt;
    if (intersection == cv::Rect())
        return 0.0f;

    float div = guess.area() + gt.area() - intersection.area();
    if (div > 0.0f)
        return intersection.area() / div;
    else
        return 0.0f;
}

template<typename T> 
bool isfinite(T arg)
{
    return arg == arg && 
           arg != std::numeric_limits<T>::infinity() &&
           arg != -std::numeric_limits<T>::infinity();
}

inline cv::Rect parseRect(std::string rep)
{
    std::replace(rep.begin(), rep.end(), ',', ' ');
    std::istringstream init_stream(rep);
    std::vector<float> coords(4, 0.0f);
    for (size_t i = 0; i < coords.size(); i++)
    {
        init_stream >> coords[i];
        if (!isfinite(coords[i]))
            coords[i] = 0.0f;
    }

    return cv::Rect(cv::Point(coords[0] + 0.5, coords[1] + 0.5),
                    cv::Point(coords[2] + 0.5, coords[3] + 0.5));
}

class GTReader
{
private:
    std::ifstream gt_file_;
    cv::Rect current_;

public:
    GTReader()
    {
    }

    GTReader(const std::string &filename)
    {
        init(filename);
    }

    bool isOpen()
    {
        return gt_file_.is_open();
    }

    cv::Rect init(const std::string &filename_or_rect)
    {
        gt_file_.open(filename_or_rect.c_str(), std::ios::in);

        if (!gt_file_.is_open())
            current_ = parseRect(filename_or_rect);

        return next();
    }

    cv::Rect next()
    {
        if (gt_file_.is_open())
        {
            std::string current_string;
            std::getline(gt_file_, current_string);
            current_ = parseRect(current_string);
        }

        return get();
    }

    cv::Rect get()
    {
        return current_;
    }
};

class PrecisionRecallEvaluator
{
private:
    int num_correct_;
    int num_responses_;
    int num_objects_;

public:
    PrecisionRecallEvaluator()
        : num_correct_(0)
        , num_responses_(0)
        , num_objects_(0)
    {
    }

    bool updateMetrics(const cv::Rect &guess, const cv::Rect &gt)
    {
        if (guess != cv::Rect())
            num_responses_++;

        if (gt != cv::Rect())
            num_objects_++;

        if (overlap(guess, gt) >= 0.25)
        {
            num_correct_++;
            return true;
        }

        return false;
    }

    std::pair<float, float> getMetrics()
    {
        return std::make_pair(
            num_responses_ == 0 ? 0.0f : num_correct_ / (float)num_responses_,
            num_objects_   == 0 ? 0.0f : num_correct_ / (float)num_objects_);
    }
};
