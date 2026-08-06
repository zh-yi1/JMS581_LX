#ifndef _LANG_H
#define _LANG_H

enum {
    /* 首页 */
    STR_HOME,
    STR_BACKUP,
    STR_SET,
    STR_REMAIN,
    STR_CARD_READY,
    STR_CARD_ABSENT,
    STR_FULL_CARD,
    STR_LATEST_7D,

    /* 保留任务 / 侧栏 / 组件 / 表盘短语 */
    STR_CLOCK,
    STR_SPORTS,
    STR_HEART_RATE,
    STR_BLOOD_OXYGEN,
    STR_BLOOD_SUGAR,
    STR_WEATHER,
    STR_TIMER,
    STR_STOP_WATCH,
    STR_CALCULATOR,
    STR_CALENDAR,
    STR_GAME,
    STR_BREATHE,
    STR_ALTITUDE,
    STR_LATEST_APP,
    STR_CALORIE,
    STR_STEPS,
    STR_DISTANCE,
    STR_BAROMETER,
    STR_TEMPERATURE,
    STR_YEAR,
    STR_MON,
    STR_MDAY,
    STR_HOUR,
    STR_MIN,
    STR_SEC,

    STR_SUNDAY,
    STR_MONDAY,
    STR_TUESDAY,
    STR_WEDNESDAY,
    STR_THURSDAY,
    STR_FRIDAY,
    STR_SATURDAY,

    STR_CLOUDY,
    STR_SUNNY,
    STR_SNOWY,
    STR_RAINY,
    STR_OVERCAST,
    STR_SAND_AND_DUST,
    STR_WINDY,
    STR_HAZE,

#if FUNC_MUSIC_EN
    STR_SD_MUSIC,
    STR_SD_MUSIC_INSET_TF,
#endif

#if FUNC_RECORDER_EN
    STR_MIC_RECORD,
#endif

    STR_NULL,
};

extern const char * const *i18n;

void lang_select(int lang_id);

#endif
