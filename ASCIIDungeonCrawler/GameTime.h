#pragma once
#include <string>

namespace DungeonGame {

    class GameTime {
    public:
        // configurable constants
        static constexpr int MINUTES_PER_HOUR = 60;
        static constexpr int HOURS_PER_DAY = 24;
        static constexpr int DAYS_PER_WEEK = 7;
        static constexpr int MONTHS_PER_YEAR = 12;
        static constexpr int DAYS_PER_MONTH = 30;
        static constexpr int MINUTES_PER_MOVE = 5;
        static constexpr int MINUTES_PER_TURN = 5;
        static constexpr int MINUTES_REST_PER_HP = 60; // 1 hour rest = 5 hp

        GameTime() = default;

        void advanceMove();   // call on player movement
        void advanceTurn();   // call on combat turn

        // getters
        int getMinute() const { return m_minute; }
        int getHour()   const { return m_hour; }
        int getDay()    const { return m_day; }
        int getMonth()  const { return m_month; }
        int getYear()   const { return m_year; }

        std::string getTimeString()  const; // "06:30"
        std::string getDateString()  const; // "Day 4, Month 2, Year 1"
        std::string getDayOfWeek()   const;

        int getTotalMinutesElapsed() const { return m_totalMinutes; }

    private:
        int m_minute = 0;
        int m_hour = 6; // start at 6AM
        int m_day = 1;
        int m_month = 1;
        int m_year = 1;
        int m_totalMinutes = 0;

        void advance(int minutes);
    };

}