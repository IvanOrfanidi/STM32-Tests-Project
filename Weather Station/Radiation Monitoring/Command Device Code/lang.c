#include "includes.h"
#include "lang.h"

const char* const l_mems[LANG_NUM][1] = {
   "Sensor",
   "Датчик",
};

const char* const l_mems_tp[LANG_NUM][7] = {
   { "(off)", "(move)", "(keep)", "(move+k)", "(turn)", "(shock)", "(crash)" },
   { "(выключен)", "(движение)", "(удержание)", "(движ+удерж)", "(переворот)", "(удар)", "(авария)" }
};

const char* const l_w_mode[LANG_NUM][2] = { { "Mode: sleep", "Mode: online" }, { "Режим: спящий", "Режим: онлайн" } };

const char* const l_ext_power[LANG_NUM][2] = { { "External power: off", "External power: on" },
                                               { "Питание: выкл.", "Питание: вкл." } };

const char* const l_gps[LANG_NUM][7] = { { "Sat:", "at", "s.", "Speed: ", "km/h", "Altitude:", "m" },
                                         { "Спутн.:", "за", "с.", "Скорость:", "км/ч", "Высота:", "м" } };

const char* const l_vbat[LANG_NUM][4] = { { "Bat.:", "V", "s.", "Warning: low power bat!" },
                                          { "Бат.:", "В", "с.", "Внимание! Низкий заряд батареи!" } };

const char* const l_misc[LANG_NUM][13] = { { "sms#",
                                             "LBS search error\n",
                                             "empty",
                                             "Command fail. GPS search time 0 min. Set GPS search time",
                                             "Command OK. Wait.",
                                             "No GPS signal",
                                             "Lang.: eng",
                                             "Name",
                                             "Statistics reset",
                                             "Network busy",
                                             "Attention!Somebody breaking password.\n",
                                             "Incorrect SMS-command.\n",
                                             "Incorrect SMS-command.\n" },
                                           { "смс#",
                                             "ошибка поиска LBS",
                                             "не задан",
                                             "Команда не выполнена, задано неверное время поиска GPS.",
                                             "Команда принята. Ждите",
                                             "Нет сигнала GPS",
                                             "Язык: русский",
                                             "Имя устр.",
                                             "Статистика сброшена",
                                             "Сеть занята",
                                             "Внимание! Попытка подбора пароля.\n",
                                             "Неправильный формат смс-команды.\n",
                                             "Неправильный формат смс-команды.\n" } };

const char* const l_event[LANG_NUM][2] = { { "Alarm! Sensor: move", "Alarm! Button press" },
                                           { "Тревога! Сенсор:движение", "Тревога! Нажата кнопка" } };

const char* const l_bbox[LANG_NUM][4] = { {
                                             "«Black box»:\ndisable.\nFLASH: ",

                                             "«Black box»:\nenable when move.\nFLASH: ",

                                             "«Black box»:\nenable.\nFLASH: ",

                                             "Recording period in a «black box»(s.): ",
                                          },
                                          {
                                             "«Черный ящик»:\nвыключен.\nFLASH: ",

                                             "«Черный ящик»:\nвключен при движении.\nFLASH: ",

                                             "«Черный ящик»:\nвключен.\nFLASH: ",

                                             "Период записи в\n«Черный ящик»(c.): ",
                                          } };
