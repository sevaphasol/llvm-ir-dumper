# LLVM IR Dumper

`LLVM IR Dumper` это LLVM pass plugin, который встраивается в `clang`, снимает два среза модуля и превращает их в наглядные графы:

- `before_opt`: состояние LLVM IR в начале optimization pipeline
- `after_opt`: состояние LLVM IR после оптимизаций

На выходе проект генерирует:

- LLVM IR в `.ll`
- граф в `.dot`
- SVG-визуализацию через Graphviz
- обычный исполняемый файл программы

## Пример дампа
Ниже можно видеть пример дампа программы [fact.c](examples/fact/fact.c) после оптимизации -O1

<p align="center">
  <img src="examples/fact/O1/svg/after_opt.svg" alt="LLVM IR graph of fact.c after optimization -O1" width="100%">
</p>

## Требования

Минимально нужны:

- `cmake`
- `python3`
- `clang`
- `LLVM` с установленным CMake-конфигом (`find_package(LLVM REQUIRED CONFIG)`)
- `graphviz` для генерации SVG

Желательно, чтобы `clang` и `LLVM`, для которого собран плагин, были одной версии.

Проект проверен в окружении с `LLVM/Clang 18.1.3`.

Если SVG не нужны, можно запускать [скрипт](scripts/compile_with_plugin.py) с `--no-svg`, тогда `graphviz` не требуется.

## Сборка

```bash
bash scripts/build.sh
```
После сборки плагин лежит здесь:

```bash
install/lib/libLLVMIRDumper.so
```

Очистка артефактов:

```bash
bash scripts/clean.sh
```

## Генерация дампа

Генерация происходит с помощью скрипта [`scripts/compile_with_plugin.py`](scripts/compile_with_plugin.py)

Этот скрипт — обёртка над `clang`, которая сама прокидывает плагин, пути для дампов и все необходимые флаги в `clang`.

Аргументы:

- `--source` путь к исходному `.c` файлу
- `--workdir` базовая директория, относительно которой трактуются все относительные пути
- `--plugin-path` путь до `libLLVMIRDumper.so`
- `--opt-level` уровень оптимизации, например `O1` или `O2`
- `--binary-out` куда положить итоговый бинарник
- `--before-ll` и `--after-ll` пути для IR-дампов
- `--before-dot` и `--after-dot` пути для `.dot`
- `--before-svg` и `--after-svg` пути для SVG
- `--no-svg` не вызывать Graphviz
- `--extra-clang-arg ARG` прокинуть дополнительный аргумент в `clang`

## Визуальная модель графа

- синяя вершина: инструкция LLVM
- красная вершина: внешний операнд, константа или значение, не являющееся инструкцией текущего блока
- светло-серый кластер: функция
- тёмно-серый вложенный кластер: `basic block`
- голубые рёбра: data flow
- красные рёбра: порядок исполнения инструкций
- оранжевые рёбра: control flow
- оранжевое пунктирное ребро: вызов функции
