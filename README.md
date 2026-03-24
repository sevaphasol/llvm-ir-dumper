# LLVM IR Dumper

`LLVM IR Dumper` это LLVM pass plugin, который встраивается в `clang`, снимает два среза модуля и сериализует их в промежуточное представление `IrGraph`:

- `before_opt`: состояние LLVM IR в начале optimization pipeline
- `after_opt`: состояние LLVM IR после оптимизаций

Пайплайн двухэтапный:

1. pass строит `IrGraph` и пишет его в JSON
2. утилита `ir_graph_to_dot` десериализует JSON и рендерит `.dot`

На выходе проект генерирует:

- LLVM IR в `.ll`
- промежуточный граф в `.json`
- граф в `.dot`
- SVG-визуализацию через Graphviz
- обычный исполняемый файл программы

## Требования

Минимально нужны:

- `cmake`
- `python3`
- `clang`
- `LLVM` с установленным CMake-конфигом (`find_package(LLVM REQUIRED CONFIG)`)
- `graphviz` для генерации SVG

Желательно, чтобы `clang` и `LLVM`, для которого собран плагин, были одной версии.

## Сборка

```bash
bash scripts/build.sh
```

После сборки устанавливаются:

```bash
install/lib/libLLVMIRDumper.so
install/bin/ir_graph_to_dot
```

## Генерация дампа

Генерация происходит с помощью [`scripts/compile_with_plugin.py`](scripts/compile_with_plugin.py).

Скрипт:

- компилирует исходник через `clang` и pass plugin
- получает `before/after` JSON-графы
- прогоняет `ir_graph_to_dot`
- при необходимости вызывает Graphviz для SVG

Аргументы:

- `--source` путь к исходному `.c` файлу
- `--workdir` базовая директория, относительно которой трактуются все относительные пути
- `--plugin-path` путь до `libLLVMIRDumper.so`
- `--converter-path` путь до `ir_graph_to_dot`
- `--opt-level` уровень оптимизации, например `O1` или `O2`
- `--binary-out` куда положить итоговый бинарник
- `--before-ll` и `--after-ll` пути для IR-дампов
- `--before-json` и `--after-json` пути для JSON-графов
- `--before-dot` и `--after-dot` пути для `.dot`
- `--before-svg` и `--after-svg` пути для SVG
- `--no-svg` не вызывать Graphviz
- `--extra-clang-arg ARG` прокинуть дополнительный аргумент в `clang`

## Архитектура

- `ir_graph` не знает ничего про DOT-атрибуты, цвета и шрифты
- pass строит только семантический `IrGraph`
- вся визуальная логика находится в `ir_graph_to_dot`
