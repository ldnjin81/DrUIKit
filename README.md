# DrUIKit

**English** | [한국어](README.ko.md)

A **style-neutral UI component** plugin for Unreal Engine 5.8. Components extend engine and CommonUI widgets by inheritance, and anything inheritance can't cover is composed inside a UserWidget. Components paint only through theme tokens; the actual look comes from a style (`UISample/<Style>`): a plugin holding a `tokens.json` that the core loads at startup. Eleven styles ship as samples: glass, flat, neo-brutalism, neumorphism and retro terminal, plus six common game UI looks: fantasy RPG, parchment, sci-fi hologram, cyberpunk neon, casual mobile and dark fantasy.

> Status: 0.1.0 (first- and second-wave components, UI flow and screen layers). Plan and progress: `Docs/PLAN.md` (Korean).

![Buttons in the glass style (light)](Docs/images/buttons-light.jpg)

## Layout

| Path | Role |
| --- | --- |
| `unreal/DrUIKit` | Core plugin (module `DrUIKit`, depends on UMG and CommonUI) |
| `Host/DrUIKitHost.uproject` | Standalone build and test host (no code module) |
| `UISample/Glass` | Liquid-glass style (plugin `DrUIGlass`: tokens, Pretendard font, backdrop, gallery level `L_DrUIGallery`) |
| `UISample/Flat`, `Brutal`, `Neu`, `Retro` | Flat/material, neo-brutalism, neumorphism and retro terminal styles (plugins `DrUIFlat`, `DrUIBrutal`, `DrUINeu`, `DrUIRetro`: tokens only, no code) |
| `UISample/Fantasy`, `Parchment`, `SciFi`, `Cyber`, `Casual`, `DarkFantasy` | Game UI styles: fantasy RPG (landscape and tavern backdrops, parchment and leather panels with ornate bronze and gold frames), parchment (scholar's desk backdrop, torn-edge paper panels, sepia ink and wax red), sci-fi hologram (translucent, cyan glow, angular), cyberpunk neon, casual mobile (thick outlines, round, solid drop shadows) and dark fantasy (cathedral and moor backdrops, near-black panels with pale-gold corner brackets). Fantasy, parchment and dark fantasy add `Images/`; the rest are tokens only |
| `Tools/verify.ps1` | Builds the host and runs `Automation RunTests DrUIKit+DrUIGlass` |

## What's included

Naming: components are `UDr<Component>` (`UDrButton`); supporting systems that aren't components are `UDrUI<…>` (`UDrUITheme`).

| Area | Classes |
| --- | --- |
| Theme | `UDrUITheme` (token data asset, grayscale default theme), `UDrUIThemeSubsystem` (current theme, switching, surface quality, surface transparency, reduce motion), `DrUIStyle` (style loader: finds style plugins, reads `tokens.json` and `style.json`), `UDrUIKitSettings` |
| State and motion | `EDrUIState` / `DrUIState::Resolve` (9 states with priority), `DrUIMotion` (ease-in/out cubic-bezier, staggered entrance `PlayStagger`), `FDrUISpring` |
| Basics | `UDrText` (UCommonTextBlock), `UDrIcon` (UImage), `UDrGlyph` (vector marks: check, dot, plus, close, arrows, exclamation), `UDrSurface` (UBorder + backdrop blur; transparent when the fill token is empty; multi-layer or hard-offset shadow and top highlight via `SetElevation`; border width follows the theme) |
| Buttons | `UDrButtonBase` (shared state framework, abstract), `UDrButton` (9 states, 4 variants) |
| Selection | `UDrToggleBase` (on/off base, abstract), `UDrCheckBox`, `UDrRadio` (single choice with `UCommonButtonGroupBase`), `UDrSwitch` (spring thumb), `UDrTabList` / `UDrTabButton` (UCommonTabListWidgetBase with a sliding pill or underline indicator), `UDrSegmentedControl`, `UDrSlider` (USlider + filled track, ticks, value bubble) |
| Status | `UDrBadge` (count, 99+, dot; 6 colors), `UDrProgressBar` (rounded, animated fill, indeterminate), `UDrProgressRing` (spring, indeterminate spin), `UDrSkeleton` (loading placeholder), `UDrSpinner` (UCircularThrobber, token color) |
| Notices | `UDrNoticeBase` (transient base, abstract), `UDrToast` (glass pill, 4 tones), `UDrToastStack` (stacking, oldest leaves first), `UDrSnackbar` (dark pill + action, one at a time with a queue, pauses on hover) |
| Input | `UDrTextField` (UEditableText + label, hint, character count, error), `UDrDropdown` / `UDrDropdownMenu` / `UDrDropdownItem` (UMenuAnchor list, focuses the chosen item first), `UDrHorizontalSelector` (◀ value ▶, gamepad left/right, wrap or clamp) |
| Gamepad | `DrUIInput` (Accept/Back from the engine's navigation config, Xbox/PlayStation/Nintendo button names, modal layer, focus memory, global handler for right-stick scrolling, LB/RB and LT/RT tab paging, button shortcuts, any-key listeners), `UDrKeyGlyph` (button shapes drawn without assets: face buttons incl. ✕○□△, bumpers, triggers, sticks, D-pad, View/Menu, keycaps, mouse), `UDrActionBar` (status line + the focused control's actions + screen-wide hints), `UDrOptionRow` (settings line: left/right belong to its control, up/down to the neighbouring line), `UDrPressAnyKey`, `UDrItemSlot` / `UDrItemGrid` (inventory grid: rarity, equipped, NEW, count; row wrap), `UDrStatCompare` (▲▼ vs. equipped), `DrUIFeedback` (UI sounds and short rumble) |
| Game | `UDrHoldButton` (hold to confirm with filling and rewinding), `UDrRadialMenu` (pie menu for mouse direction, gamepad stick and keyboard; spring highlight), `UDrCooldown` (clockwise veil, remaining time, flash on ready), `UDrFeed` / `UDrFeedEntry` (pickup log, merges repeats as ×N) |
| UI flow | `UDrUIFlowSubsystem` (local player subsystem: creates screen layers; title, HUD, pause, settings, back, input mode, confirm dialogs, notices), `UDrUIRootLayout` (game, menu, modal and notice layers), `UDrUIMenuWidget` / `UDrUIHUDWidget` (menu and HUD bases), `UDrConfirmDialog` (modal confirm). Configured under "Flow" and "Input" in `UDrUIKitSettings` |
| Extended | `UDrScrollBox` (thin bar, edge shading), `UDrCountUp`, `UDrAnimatedSwitcher` (fade, slide, zoom; instant under reduce motion), `UDrExpandableArea` / `UDrExpanderHeader` (spring height), `UDrTextArea`, `UDrMorphButton` (shrinks to a circle while loading, expands after a check), `UDrQuantityStepper` / `UDrIconButton` (− value +, accelerating repeat on hold), `UDrChip` (filter, choice and input chips) |
| Navigation | `UDrSidebar` / `UDrSidebarItem` (rail ↔ expanded, glass lens), `UDrDock` / `UDrDockItem` (magnify on hover), `UDrBottomTabBar` / `UDrBottomTabButton`, `UDrBanner` (inline notice), `UDrBottomSheet` (drag to dismiss), `UDrCoachMark` / `UDrCoachScrim` (spotlight + step bubble), `UDrPopover` / `UDrPopoverPanel` |
| Expression | `UDrStepIndicator`, `UDrCompletion` (drawn check), `UDrEmptyState` (reason + next action), `UDrTextReveal` (typing, scramble, rise), `UDrSparkline` / `UDrDonut` / `UDrStatCard` (data display) |
| Discovery | `UDrTooltip` (glass bubble in the engine tooltip window, title + body, `Attach`), `UDrListRow` (hover lens, selection, focus; works in button groups or as a UCommonListView entry, which needs a Widget Blueprint subclass in the editor), `UDrListItem`, `UDrPageIndicator` (current dot stretches into a pill), `UDrButtonPrompt` (keycap chip + action, switches between keyboard and gamepad, hold progress) |

## Gamepad

Every component can be driven with a gamepad or keyboard alone.

- Focus rings appear for gamepad/keyboard focus only; a mouse click doesn't leave a ring. Overlays (dialogs, popovers, sheets, dropdowns, coach marks) remember where they were opened from and give focus back when closed, and menus return to the last focused item.
- Accept and Back follow the engine's navigation config, so platforms that swap them are handled. Sliders adjust with the D-pad without locking first; scroll boxes follow focus and scroll with the right stick. Tab lists page with `Paging` = Bumpers (LB/RB) or Triggers (LT/RT), show those buttons at both ends, and Down from a tab enters its content. Components can set their own neighbours (`NavTargets`, `DrUIInput::ChainVertical`), which works for the D-pad and the stick alike; while an overlay is open, layers below don't react to navigation.
- **Inline button prompts.** Give a button `SetShortcut(KeyboardKey, GamepadKey)` and the key shape appears inside it. The key presses it from anywhere on the screen; when an overlay is open, only buttons inside that overlay respond.
- **Key shapes.** Button prompts and the action bar draw real key shapes for Xbox, PlayStation or Nintendo pads. Auto mode follows the connected pad; `Both` shows the pad and keyboard keys side by side. A plugin with `Glyphs/glyphs.json` (key name → PNG per Xbox/PlayStation/Nintendo/keyboard set) replaces the vector shapes with images, loaded at runtime without assets (`DrUIGlyphImages`). For a console release, build such a set from the platform's official glyphs (available from each platform's developer portal) in a private plugin; the public sample `UISample/Prompts/DrUIPromptsKenney` uses Kenney Input Prompts (CC0; not official platform art). White glyphs are tinted with theme colors (`"tint"`), `<pad>_brand` sets supply the brand-colored face buttons, and keys a set lacks (here the D-pad) stay vector. Like style plugins, a packaged game must stage the `Glyphs/` folder.
- **Mouse vs. pad.** `DrUIInput::GetLastInputKind()` tells pointer, keyboard and gamepad apart. The mouse reaches anything directly without walking depths; the pad and keyboard follow focus. When you switch back to the pad, focus resumes where the mouse was (or the last pad position).
- **Accessibility and feedback.** Text size scale (`SetTextScale`), UI sounds (set in project settings) and short rumble, each of which can be switched off.

**Console game menu sample** (`UDrGlassConsoleMenu`, opened from the gallery's Gamepad page or with View/M) shows a typical pad menu layout:

- Main tabs (LB/RB) and sub-tabs (LT/RT) along the top.
- Three columns: a first-level list, a second-level list and a details panel with a stat comparison.
- A status bar along the bottom with the available actions.
- Pad: lists wrap and never let focus leave the menu. Right/A goes into the second level (back to the row you were on), Left/B goes back, and paging moves focus to the first level. In the second level A equips and X drops.
- The Settings tab fills the second level with option rows (selector, slider, segments, switch, check box, radios, stepper, dropdown, text field, hold button, dialog button), so every kind of left/right control can be tried there.
- Mouse: click any row at any depth, hover to preview, right-click or Esc to close.

| Console game menu | Gamepad page (PlayStation glyphs) |
| --- | --- |
| ![Console game menu](Docs/images/console-menu-light.jpg) | ![Gamepad page](Docs/images/gamepad-light.jpg) |

## Styles

A style is any enabled plugin with `Tokens/tokens.json` (design tokens, light and dark) and `Tokens/style.json` (id, name, fonts, which fills follow the transparency setting, base border width). The core loads every style at startup; theme ids are `<style>-light` and `<style>-dark`. Fonts are TTF paths: `{plugin}`, `{plugin:Name}` or `{engine}`, plus an optional `hangul` typeface for Korean glyphs. `style.json` can also map tokens to images under `images`: `ground` is the screen backdrop (`fit: cover`, keeps aspect and fills), any other token paints surfaces filled with that token as a 9-slice (`margin` in image pixels, `scale` for on-screen size; the frame shrinks to fit thin surfaces). `{mode}` in the path becomes `light` or `dark`, and PNG/JPG files are turned into textures on first use (no assets). Image surfaces skip backdrop blur and the top highlight. To ship a style in a packaged game, stage its `Tokens/` (and fonts and `Images/`) with `RuntimeDependencies`, as `DrUIGlass` does.

| Flat (light) | Neo-brutalism (light) |
| --- | --- |
| ![Flat](Docs/images/style-flat-light.jpg) | ![Neo-brutalism](Docs/images/style-brutal-light.jpg) |
| Neumorphism (light) | Retro terminal (dark) |
| ![Neumorphism](Docs/images/style-neu-light.jpg) | ![Retro terminal](Docs/images/style-retro-dark.jpg) |
| Fantasy RPG (dark) | Sci-fi hologram (dark) |
| ![Fantasy RPG](Docs/images/style-fantasy-dark.jpg) | ![Sci-fi hologram](Docs/images/style-scifi-dark.jpg) |
| Cyberpunk neon (dark) | Casual mobile (light) |
| ![Cyberpunk neon](Docs/images/style-cyber-dark.jpg) | ![Casual mobile](Docs/images/style-casual-light.jpg) |
| Dark fantasy (dark) | Parchment (light) |
| ![Dark fantasy](Docs/images/style-darkfantasy-dark.jpg) | ![Parchment](Docs/images/style-parchment-light.jpg) |

## Gallery

Open `L_DrUIGallery` from `UISample/Glass` (the host's default map) to see every component on tabbed pages; the dropdown next to the tabs changes the style. When you add a component, add it to a gallery page and to `UDrGlassGallery::CoveredClasses()` in the same change; otherwise `DrUIGlass.Gallery.CoversAllKitWidgets` fails. Running with `-game -DrUIGalleryCapture -RenderOffscreen` captures every style × light/dark × page into `Saved/Screenshots` and exits (`-DrUIGalleryStyles=glass,flat` limits the styles). `-DrUIGamepadTour` instead feeds synthetic gamepad input through the Gamepad page and the console game menu and captures each step (`DrUITour_*.png`).

| Selection (dark) | Game (pie menu, cooldown, feed) |
| --- | --- |
| ![Selection](Docs/images/selection-dark.jpg) | ![Game](Docs/images/game-light.jpg) |
| Discovery (page dots, button prompts) | Confirm dialog (modal layer) |
| ![Discovery](Docs/images/navigation-dark.jpg) | ![Confirm dialog](Docs/images/dialog-light.jpg) |
| Expression (empty state, text reveal, data) | |
| ![Expression](Docs/images/expression-dark.jpg) | |

The sample's labels are in Korean; the components themselves take any text.

## Verify

```powershell
.\Tools\verify.ps1      # default engine path: C:\Program Files\Epic Games\UE_5.8
```

## License

MIT
