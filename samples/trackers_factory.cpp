#include "tracker.hpp"

cv::Ptr<Tracker> createTrackerDummy();
cv::Ptr<Tracker> createTrackerMoskalenko();

cv::Ptr<Tracker> createTracker(const std::string &impl_name)
{
    if (impl_name == "dummy")
        return createTrackerDummy();
    else if (impl_name == "moskalenko")
        return createTrackerMoskalenko();
    return 0;
}
