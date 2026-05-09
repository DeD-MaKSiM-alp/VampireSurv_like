# Наброски UI

Документ фиксирует выбранные принципы и описывает каркас UI-слоя на
основу, без полной перестройки текущих экранов. Не реализация.

---

## 1. Цели итерации

- Заложить пайплайн: «указал путь к картинке → она стала иконкой
  такого-то элемента UI» без правок в коде и без пересборки.
- Заложить масштабирование, чтобы один и тот же экран корректно
  выглядел на 1280x720, 1920x1080, 2560x1440, ultrawide 21:9.
- Сделать один полноценный UI-элемент (карточка перка) как
  доказательство пайплайна.
- Не трогать сейчас текущий run-HUD / base / stop overlay / result
  overlay — они мигрируют позднее.

## 2. Принципы

- **Один источник истины для размеров — логические пиксели.** Все
  числа в коде UI выражены в логической системе. Реальная отрисовка
  получает их умноженными на `scale`.
- **Всё, что показывается игроку, имеет идентификатор в манифесте.**
  Шрифты, иконки, рамки, фоны — внутри кода ссылаются на ID, не на
  путь файла. Замена `assets/icons/perks/power.png` → другая картинка
  без правок в `Perks.cpp`.
- **Никаких хардкод-позиций «x=64, y=140 в пикселях окна».** Только
  `anchor + offset` в логических координатах.
- **Старый sf::Text / sf::RectangleShape не запрещены** — это
  нижележащий слой. Над ним лежит UI-слой, который ими пользуется.

## 3. Зафиксированные решения

| Параметр | Значение |
|---|---|
| Арт-стиль | 2D-векторный / высокоразрешённый (smooth) |
| Фильтрация текстур UI | bilinear (`sf::Texture::setSmooth(true)`) |
| Логическое разрешение | **1920x1080** |
| Стратегия масштабирования | Логический size + якоря, без леттербокса |
| Опорная сторона для scale | По высоте окна |
| Манифест ассетов | Внешний текстовый файл `assets/ui.txt` |
| Шрифты | Семейство: `title` + `body` |
| Fallback при отсутствии файла | Маджента-клетка (как сейчас) + сообщение в stderr |
| Объём итерации | Каркас (AssetManager + scaling + 1-2 элемента-образца) |

## 4. Логическая координатная система

- Origin: верхний-левый угол экрана.
- Оси: X вправо, Y вниз (как в SFML).
- Размеры и offset элементов выражаются в логических пикселях так,
  как если бы окно было ровно 1920x1080.

## 5. Масштабирование

```
const float scale = static_cast<float>(window.getSize().y) / 1080.0f;
const float logicalScreenW = static_cast<float>(window.getSize().x) / scale;
```

- Любой элемент с логическим размером `(w_l, h_l)` рисуется с реальным
  размером `(w_l * scale, h_l * scale)`.
- Шрифты: `setCharacterSize(static_cast<unsigned>(logicalSize * scale))`.
- На ultrawide экране `logicalScreenW` будет больше 1920 — это
  нормально. HUD остаётся прижат к якорям, а игровое поле получает
  больше места по горизонтали.

### Якоря

```
enum class Anchor : uint8_t {
    TopLeft,    TopCenter,    TopRight,
    MidLeft,    Center,       MidRight,
    BotLeft,    BotCenter,    BotRight
};
```

Координата элемента вычисляется как:

```
anchorScreenPos = anchorOf(window, logicalScreenW, anchor)
finalPos        = anchorScreenPos + offset * scale
```

Где `offset` — двумерный вектор в логических пикселях, относительно
выбранного якоря. Это даёт нативную поддержку любого aspect ratio: HUD
прижат к углам, центр-якорные оверлеи всегда по центру.

## 6. Манифест ассетов (`assets/ui.txt`)

Текстовый файл, парсится при старте `Application`. Формат —
по одному правилу на строку:

```
# Comments start with # and are ignored.
# Empty lines are ignored.
# Format: <id> <space> <relative_path>

# Fonts
font.title          assets/fonts/title.ttf
font.body           assets/fonts/body.ttf

# Perk icons
icon.perk.power_i           assets/icons/perks/power_i.png
icon.perk.attack_speed_i    assets/icons/perks/attack_speed_i.png
icon.perk.vitality_i        assets/icons/perks/vitality_i.png
icon.perk.calm_mind         assets/icons/perks/calm_mind.png
icon.perk.momentum          assets/icons/perks/momentum.png
icon.perk.range_i           assets/icons/perks/range_i.png

# UI primitives
ui.panel.frame      assets/ui/frame.png
ui.panel.bg         assets/ui/panel_bg.png
```

Правила:

- Идентификатор — латиница, точки и подчёркивания. Точки только для
  иерархии: `category.subcategory.name`.
- Путь относительно корня проекта (рядом с `.exe` есть `assets/`).
- Неизвестные ID при чтении из кода → fallback (см. §9) + stderr.
- Дубликаты ID — последний выигрывает + stderr предупреждение.
- Hot-reload в Debug — желательно, но не в этой итерации.

## 7. Расширение AssetManager

Текущий `AssetManager` уже умеет загружать sprite-текстуры по ключу.
Расширение:

- `loadManifest("assets/ui.txt")` — читает манифест, для каждой пары
  `(id, path)` вызывает соответствующий `loadXxx`.
- Различает по префиксу:
  - `font.*` → `sf::Font` cache.
  - всё остальное (`icon.*`, `ui.*`) → `sf::Texture` cache.
- Геттеры:
  - `const sf::Texture& textureOrFallback(id)`
  - `const sf::Font* font(id)` (может быть null, если manifest не
    подгрузил → элементу решать, рисовать ли текст вообще).
- Все загружаемые UI-текстуры получают `setSmooth(true)`.
- Игровые текстуры (`player`, `enemy_*`, `projectile_*`, `pickup_*`)
  пока остаются на старом пути загрузки в `RunScreen`. Их миграция —
  следующая итерация.

## 8. Шрифты

| ID манифеста | Назначение | Логический размер по умолчанию |
|---|---|---|
| `font.title` | Заголовки, баннеры, акценты | 36 px |
| `font.body` | Описания, инструкции, HUD-текст | 22 px |

Реальный pixel size = `logicalSize * scale` пересчитывается каждый
кадр (или при ресайзе — оба варианта дёшевы).

## 9. Fallback политика

При первом обращении к ID, который не разрешён в манифесте или чей
файл не загрузился:

1. Возвращается заведомо «громкая» текстура — текущая маджента-чёрная
   шахматная клетка из `AssetManager::createFallbackTexture()`.
2. В `std::cerr` пишется один раз для этого ID:
   `[ui] missing asset 'icon.perk.power_i' (path: assets/icons/perks/power_i.png)`.
3. Игра продолжает работать. Это позволяет доводить ассеты
   итеративно, не ломая билды на половине пути.

Аналогично для шрифтов: если `font.title` не найден — UI-элемент
пропускает отрисовку текста (с одной строкой в stderr) и не падает.

## 10. Образец итерации: карточка перка в Level-up overlay

Карточка — самостоятельный UI-элемент `PerkCard`. Логические размеры:

```
Width:   320 px
Height:  400 px

Layout (top to bottom):
  +------------------------+
  |  [number badge]        |   24 px от верха, 36 px размер
  |                        |
  |   +---- icon ----+     |   icon 96x96, центр по горизонтали
  |   |              |     |
  |   +--------------+     |
  |                        |
  |   Power I              |   title font, центр, 60 px от иконки
  |                        |
  |   +15% damage          |   body font, центр, 30 px от названия
  +------------------------+
```

Конструктор/инициализация:

```cpp
PerkCard card;
card.setIcon("icon.perk.power_i");
card.setTitle("Power I");
card.setDescription("+15% damage");
card.setBadge("1");

card.setAnchor(Anchor::Center);
card.setOffset({-360.0f, 0.0f});  // -360 / 0 / +360 для трёх карточек
```

При render(): берёт нужный `sf::Texture` из AssetManager по
`iconId`, центрирует и масштабирует. Текст — через шрифты `font.title`
/ `font.body`.

В первой итерации overlay выбора перка перерисовывается через три
`PerkCard`. Существующая текстовая «1) … 2) … 3) …» строка
заменяется на три карточки. Остальной overlay (полупрозрачный rect и
заголовок «Level Up! Choose 1 of 3») остаётся как есть и переезжает
позднее.

## 11. Новые файлы и каталоги

```
assets/
  ui.txt                     # манифест
  fonts/
    title.ttf                # пока fallback на текущий arial
    body.ttf
  icons/
    perks/
      power_i.png
      attack_speed_i.png
      vitality_i.png
      calm_mind.png
      momentum.png
      range_i.png
  ui/
    (зарезервировано под рамки/панели позднее)

src/ui/
  Anchor.h                   # enum + helper anchorOf(window, w_logical, anchor)
  UiScale.h/.cpp             # scale = window.h / 1080, logicalScreenW
  UiManifest.h/.cpp          # парсер ui.txt
  UiTheme.h                  # FontSize::Title = 36, FontSize::Body = 22
  PerkCard.h/.cpp            # образец-элемент
```

`AssetManager` остаётся в `src/assets/`, дополняется методами
`loadManifest` и `font(id)`.

## 12. Что НЕ делается в этой итерации

- Полная замена sf::Text/RectangleShape во всех экранах.
- Run-HUD остаётся как есть.
- Stop overlay / Result overlay / Defeat overlay остаются как есть.
- Игровые ассеты (`player`, `enemy_*`, `projectile_*`, pickups) не
  мигрируют в манифест, остаются на текущем pipeline загрузки.
- Никаких 9-slice панелей, теневых рамок, hover/click эффектов.
- Никакого настройщика разрешения внутри игры.
- Hot-reload манифеста.

## 13. Открытые хвостовые вопросы

1. Откуда берём 6 иконок перков в первой итерации? Варианты:
   - временные placeholder-иконки (например, цветной круг с буквой);
   - готовые иконки из публичных библиотек ассетов;
   - заказ дизайнеру / отрисовка вручную.
2. Шрифты `title.ttf` / `body.ttf` — выбираем сейчас конкретные или
   используем текущий `arial` для обоих с пометкой TODO?
3. После того, как карточка перка станет работающим прецедентом —
   какой следующий элемент мигрирует: HP/Arousal полосы, базовая
   панель, кнопки на base?
4. Когда переходить с stub-иконок на финальные — чтобы вкомитить
   итерацию-каркас уже с заглушками или вкомитить сразу с
   placeholder-иконками генерации (например, цветной круг с буквой
   названия)?
