#include "GameTime.h"
#include <sstream>
#include <iomanip>

namespace DungeonGame {

    static const char* DAY_NAMES[] = {
        "Monday", "Tuesday", "Wednesday",
        "Thursday", "Friday", "Saturday", "Sunday"
    };

    static const char* MONTH_NAMES[] = {
        "Deepwinter", "Thawmelt", "Seedrise",
        "Greengrow", "Brightmoon", "Highsun",
        "Harvestide", "Emberdusk", "Ashfall",
        "Frostmark", "Dimtide", "Yearend"
    };

    void GameTime::advance(int minutes) {
        m_totalMinutes += minutes;
        m_minute += minutes;

        while (m_minute >= MINUTES_PER_HOUR) {
            m_minute -= MINUTES_PER_HOUR;
            m_hour++;
        }
        while (m_hour >= HOURS_PER_DAY) {
            m_hour -= HOURS_PER_DAY;
            m_day++;
        }
        while (m_day > DAYS_PER_MONTH) {
            m_day -= DAYS_PER_MONTH;
            m_month++;
        }
        while (m_month > MONTHS_PER_YEAR) {
            m_month -= MONTHS_PER_YEAR;
            m_year++;
        }
    }

    void GameTime::advanceMove() { advance(MINUTES_PER_MOVE); }
    void GameTime::advanceTurn() { advance(MINUTES_PER_TURN); }
    void GameTime::advanceHour() { advance(MINUTES_PER_HOUR); }


    std::string GameTime::getTimeString() const {
        std::ostringstream ss;
        ss << std::setw(2) << std::setfill('0') << m_hour
            << ":"
            << std::setw(2) << std::setfill('0') << m_minute;
        return ss.str();
    }

    std::string GameTime::getDateString() const {
        return "Day " + std::to_string(m_day)
            + " " + MONTH_NAMES[m_month - 1]
            + " Y" + std::to_string(m_year);
    }

    std::string GameTime::getDayOfWeek() const {
        int dayIndex = ((m_day - 1) + (m_month - 1) * DAYS_PER_MONTH) % DAYS_PER_WEEK;
        return DAY_NAMES[dayIndex];
    }

}