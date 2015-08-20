#include "tracker.hpp"

cv::Ptr<Tracker> createTrackerDummy();
cv::Ptr<Tracker> createTrackerShchedrin();

cv::Ptr<Tracker> createTracker(const std::string &impl_name)
{
    if (impl_name == "dummy")
        return createTrackerDummy();
    else if (impl_name == "Shchedrin")
        return createTrackerShchedrin();

    return 0;
}
