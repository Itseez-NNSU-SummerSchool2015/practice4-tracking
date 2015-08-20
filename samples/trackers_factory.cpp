#include "tracker.hpp"

cv::Ptr<Tracker> createTrackerDummy();
// TODO: Declare your implementation here
cv::Ptr<Tracker> createTrackerKoryukina();

cv::Ptr<Tracker> createTracker(const std::string &impl_name)
{
    if (impl_name == "dummy")
        return createTrackerDummy();
    // TODO: Add case for your implementation
    else if (impl_name == "koryukina")
        return createTrackerKoryukina();

    return 0;
}
