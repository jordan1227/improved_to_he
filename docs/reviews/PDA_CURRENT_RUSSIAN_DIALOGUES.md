# Current Russian PDA dialogues

Source-only review rendering. Conditions and actions are shown for review; this is not an apply-ready patch.

## he_pda_sidor_inquiry

- **0**: Сидорович, ПДА у тебя найдётся? Не чужой, с паролем, а нормальный.
  next: 1; next: 21

- **1**: Найдётся. Пятнадцать тысяч. Чистый, исправный, к сети подключается. Чужие тайны по покойникам ищут, а за рабочей вещью приходят ко мне.
  precondition: he_pda_access.sidor_initial_offer_available; next: 2; next: 10; next: 30; next: 40

- **2**: Ладно. Вот пятнадцать тысяч, беру.
  precondition: he_pda_access.base_purchase_available; action: he_pda_access.buy_base; next: 3; next: 900; next: 4

- **3**: Договорились. Сейчас проверю аппарат.
  precondition: he_pda_access.last_transaction_pending; action: dialogs.break_dialog

- **4**: Погоди. Не сходится. Проверь, всё ли у тебя на месте.
  precondition: he_pda_access.last_transaction_failed; action: dialogs.break_dialog

- **10**: А подешевле никак? Может, старый переделать?
  action: he_pda_access.mark_sidor_discount; next: 21

- **20**: Подумаю. Пока не надо.
  action: dialogs.break_dialog

- **21**: Ладно, погоди. Десять тысяч. И не смотри так, ниже уже некуда. На чужой аппарат ещё мастера найди, потом за работу расплатись. А этот бери и пользуйся.
  precondition: he_pda_access.sidor_discount_offer_available; next: 12; next: 13; next: 30; next: 20

- **12**: Ладно. Десять тысяч, и беру.
  precondition: he_pda_access.sidor_base_purchase_available; action: he_pda_access.buy_sidor_base; next: 3; next: 900; next: 4

- **13**: Десять тоже не мелочь. А кто за старый возьмётся?
  next: 14

- **14**: Тогда к Вергасу. Он тут неподалёку и с такими вещами возится. Если возьмётся, соберёт рабочий аппарат из того, что принесёшь.
  next: 20

- **30**: Ты ведь смотрел тот ПДА, с которым меня привезли. Что там было?
  precondition: he_pda_access.sidor_intro_available; next: 31

- **31**: Одну надпись запомнил: «Убить Стрелка». Ты мне тогда ни себя объяснить не мог, ни кто такой Стрелок. Я чужие поручения не разбираю, Меченый. Сначала на ноги встань, потом выясняй, кто тебе это написал.
  next: 32

- **32**: А сам аппарат? Его можно было починить?
  next: 33

- **33**: Нет. Я поручил отдать его Вергасу. Не знаю, что он там с ними делает, но из той коробки рабочей вещи уже не получить. А вот тебе бы рабочая коробочка пригодилась, хе-хе... У меня такая есть. Смекаешь?
  action: he_pda_access.mark_sidor_intro_seen; next: 34; next: 35

- **34**: Понял. Тогда давай о рабочем ПДА.
  next: 1; next: 21

- **35**: Вергас? Это кто такой?
  precondition: he_pda_access.vergas_not_met; next: 36

- **36**: Местный умелец. В деревне новичков живёт, в одном из домов устроился. У сталкеров железо ломается, у него работа не переводится. Загляни к нему на огонёк, познакомься. Не бойся, старый пёс не кусается. Если, конечно, не станешь учить его паяльник держать.
  next: 34

- **40**: Подумаю. Пока не надо.
  action: he_pda_access.mark_sidor_discount; next: 21

- **900**: Договорились. Сейчас проверю аппарат.
  precondition: he_pda_access.last_transaction_success; action: dialogs.break_dialog

## he_pda_barman_purchase

- **0**: ПДА купить можно? Мне свой нужен.
  next: 1

- **1**: Есть один исправный. Восемь тысяч, батарея полная. Настрою и проверю при тебе, чтобы с ним не было сюрпризов.
  next: 2; next: 10; next: 20

- **2**: Восемь тысяч. Держи, беру.
  precondition: he_pda_access.barman_purchase_available; action: he_pda_access.buy_barman_base; next: 3; next: 900; next: 4

- **3**: Хорошо. Подожди немного, я его проверю.
  precondition: he_pda_access.last_transaction_pending; action: dialogs.break_dialog

- **4**: Погоди. Не сходится. Проверь, всё ли у тебя на месте.
  precondition: he_pda_access.last_transaction_failed; action: dialogs.break_dialog

- **10**: А если старый найти? Его можно у кого-нибудь переделать?
  next: 11

- **11**: Старый ПДА лучше покажи Вергасу. Он умеет их перебирать. Тут я продаю готовые, а в чужую начинку не лезу.
  next: 20

- **20**: Подумаю. Пока не надо.
  action: dialogs.break_dialog

- **900**: Хорошо. Подожди немного, я его проверю.
  precondition: he_pda_access.last_transaction_success; action: dialogs.break_dialog

## he_pda_sidor_upgrade

- **0**: Слышал, у тебя новые ПДА появились. Чем лучше обычных?
  next: 1

- **1**: Партия небольшая, но удачная. Батарея живёт вдвое дольше, на карте можно оставлять свои пометки. А если рядом есть включённый радиометр, ПДА покажет и его показания. За один возьму два артефакта второго уровня.
  next: 2; next: 10

- **2**: Подходит. Забирай два артефакта.
  precondition: he_pda_access.upgrade_artifacts_have; action: he_pda_access.upgrade_pda; next: 3; next: 900; next: 4

- **3**: Договорились. Сейчас проверю аппарат.
  precondition: he_pda_access.last_transaction_pending; action: dialogs.break_dialog

- **4**: Погоди. Не сходится. Проверь, всё ли у тебя на месте.
  precondition: he_pda_access.last_transaction_failed; action: dialogs.break_dialog

- **10**: Подумаю. Пока не надо.
  action: dialogs.break_dialog

- **900**: Договорились. Сейчас проверю аппарат.
  precondition: he_pda_access.last_transaction_success; action: dialogs.break_dialog

## he_pda_vergas_reflash

- **0**: Чужой ПДА под меня переделать сможешь?
  next: 1

- **1**: Если плата живая, соберу. Один чужой ПДА и простой артефакт первого уровня - получится один рабочий аппарат. Или пять ПДА без артефакта: часть разберу на запчасти. Разряженный тоже годится, заряжу сам. Только записи с отданных аппаратов не переживут перепрошивку.
  next: 10; next: 20; next: 40; next: 30

- **10**: Один ПДА и простой артефакт. Давай выберем.
  precondition: he_pda_access.can_vergas_one; action: he_pda_access.preview_vergas_one; next: 11

- **20**: Отдам пять ПДА. Покажу, какие.
  precondition: he_pda_access.can_vergas_five; action: he_pda_access.preview_vergas_five; next: 11

- **11**: Номера проверь сам. Помечай те, что отдашь. Передумаешь - снимешь пометку.
  next: 12; next: 13; next: 14; next: 30

- **12**: Этот пометить или убрать?
  action: he_pda_access.select_donor; next: 11

- **13**: Покажи следующий.
  action: he_pda_access.next_donor; next: 11

- **14**: Выбрал. Отдаю отмеченные.
  precondition: he_pda_access.selection_ready; next: 15

- **15**: Хорошо. Список перед тобой. И ещё раз: записи я не сохраняю. Всё нужное с этих ПДА уже прочитал?
  next: 16; next: 30

- **16**: Да. Делай.
  precondition: he_pda_access.selection_ready; action: he_pda_access.confirm_reflash; next: 17; next: 900; next: 18

- **17**: Тогда подожди. Проверю подключение.
  precondition: he_pda_access.last_transaction_pending; action: dialogs.break_dialog

- **18**: Погоди. Не сходится. Проверь, всё ли у тебя на месте.
  precondition: he_pda_access.last_transaction_failed; action: dialogs.break_dialog

- **30**: Подумаю. Пока не надо.
  action: he_pda_access.clear_vergas_preview; action: dialogs.break_dialog

- **40**: А этот артефакт тебя чем не устраивает?
  precondition: he_pda_access.vergas_artifact_too_poor; next: 41

- **41**: Посмотри, сколько в нём жизни осталось. Мне он нужен для аппаратуры, не на полку. Принесёшь такой же, но посвежее - поговорим.
  next: 30

- **900**: Тогда подожди. Проверю подключение.
  precondition: he_pda_access.last_transaction_success; action: dialogs.break_dialog

## he_pda_sidor_burn

- **0**: ПДА после выброса сгорел. Есть чем заменить?
  next: 1

- **1**: Обычных больше нет. Что было, разобрали, а когда ещё привезут, я тебе не обещаю. Со сгоревшим к Доценту сходи. Он в этой начинке понимает: может, твой поднимет, может, замену подберёт.
  next: 2

- **2**: Понятно. Поговорю с ним.
  action: dialogs.break_dialog

## he_pda_barman_burn

- **0**: ПДА после выброса сгорел. Есть чем заменить?
  next: 1

- **1**: Обычных больше нет. Со сгоревшим лучше к Доценту. Он скажет, можно ли поднять аппарат, или предложит замену.
  next: 2

- **2**: Понятно. Поговорю с ним.
  action: dialogs.break_dialog

## he_pda_vergas_module

- **0**: Вергас, посмотри, что нашёл. К ПДА такое подойдёт?
  next: 1

- **1**: Знакомая плата. Похоже, от полевого навигатора: на корпусе бумага с маршрутами и карандашными поправками. Кто хозяин и откуда она, я не скажу. Разъём похож на ПДА, но разводка другая. Напрямую воткнёшь - спалишь и плату, и аппарат.
  next: 2; next: 20

- **2**: А ты сможешь приспособить?
  next: 3

- **3**: Смогу. Переходник соберу, прошивку подправлю - будешь сам ставить пометки на карте и подписывать их. В Зоне память подводит чаще, чем железо, так что лишняя отметка не помешает. За работу возьму один простой артефакт первого уровня с остатком не меньше половины ресурса. Плата останется внутри, заберёшь ПДА уже с ней. Сделаю при тебе.
  next: 4; next: 10; next: 20

- **4**: Договорились. Вот плата и артефакт. ПДА отдаю на работу.
  precondition: he_pda_access.module_install_available; action: he_pda_access.install_notes_module; next: 5; next: 900; next: 6

- **5**: Клади сюда. Только за провода не хватайся, пока проверяю.
  precondition: he_pda_access.last_transaction_pending; next: 21

- **6**: Погоди. Не сходится. Проверь, всё ли у тебя на месте.
  precondition: he_pda_access.last_transaction_failed; next: 20

- **10**: Чего тебе не хватает для работы?
  precondition: he_pda_access.module_install_unavailable; next: 11

- **11**: Нужны сама плата, обычный рабочий заряженный ПДА и простой артефакт первого уровня с остатком не меньше половины ресурса. Пустой артефакт не возьму. Если ПДА сейчас у мастера, сначала забери его.
  next: 20

- **20**: Подумаю. Пока не надо.
  action: dialogs.break_dialog

- **21**: Хорошо, не буду мешать.
  action: dialogs.break_dialog

- **900**: Клади сюда. Только за провода не хватайся, пока проверяю.
  precondition: he_pda_access.last_transaction_success; next: 21

## he_pda_sidor_sold_out

- **0**: Слышал, у тебя новые ПДА появились. Чем лучше обычных?
  next: 1

- **1**: Появились, да кончились. Как Выжигатель затих, все на север засобирались. Кто за хабаром, кто за Монолитом, а снаряжение всем подавай. Разобрали как пирожки на вокзале! У Доцента спроси, он себе кое-что придержал. Только учти, у него свой расчёт.
  next: 2

- **2**: Понял. Значит, к Доценту схожу.
  action: dialogs.break_dialog

## he_pda_docent_upgrade

- **0**: Доцент, ПДА получше обычного у тебя найдётся?
  next: 1

- **1**: Найдётся, Меченый. После Выжигателя все вдруг решили, что без нового аппарата на север ни шагу. А там и со старым пропасть недолго... Этот подольше проживёт на одной зарядке, пометки на карте оставлять позволяет, показания включённого радиометра выводит. Начинка от помех чуть лучше прикрыта, только от выброса я тебе гарантии не дам. Принесёшь одну капсулу, и забирай. Любую, какая есть.
  next: 2; next: 10; next: 20

- **2**: Вот капсула. Давай аппарат.
  precondition: he_pda_access.docent_upgrade_available; action: he_pda_access.buy_docent_upgrade; next: 3; next: 4; next: 5

- **3**: Ладно, сейчас подготовлю. На севере пригодится, только голову им не заменишь.
  precondition: he_pda_access.last_transaction_pending; action: dialogs.break_dialog

- **4**: Погоди. Не сходится. Проверь, всё ли у тебя на месте.
  precondition: he_pda_access.last_transaction_failed; action: dialogs.break_dialog

- **5**: Готово. Забирай. На севере пригодится, только голову им не заменишь.
  precondition: he_pda_access.last_transaction_success; action: dialogs.break_dialog

- **10**: Что мне принести? Или ещё что-то надо уладить?
  precondition: he_pda_access.docent_upgrade_unavailable; next: 11

- **11**: Одну капсулу. Сгоревший обычный ПДА можешь заменить этим, отдельно за ремонт платить не придётся. Но если уже отдал аппарат в ремонт или на зарядку, сначала забери его. Не будем два дела вперемешку делать.
  next: 20

- **20**: Подумаю. Пока не надо.
  action: dialogs.break_dialog
