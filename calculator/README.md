# Calculator

В этой задаче вам предстоит написать калькулятор в ООП стиле.

## Немного теории

***Def*** Токен - значимая неделимая единица выражения. Примеры: 1. +, )

***Def*** Выражение - набор токенов

***Def*** Корректное выражение - выражение корректное с точки зрения математики (правильная последовательность токенов)

***Def*** [Польская нотация](https://en.wikipedia.org/wiki/Polish_notation) - форма записи математических выражений в которой сначала пишутся операторы а затем операнды

Выражения записанные в польской нотации гораздо проще для разбора так как операции в ней уже записаны в нужном
для вычисления порядке.

*Пример:*

`"+ 3 4": извлекаем оператор + -> он бинарный -> вычисляем следующие два операнда 3 и 4 -> получаем результат операции 7`

`"+ 3 + 4 5": извлекаем оператор + -> он бинарный -> вычисляем следующие два операнда -> 3 получается тривиально -> второй операнд - результат операции + -> вычисляем его аналогично первому пункту (9) -> вычисляем результат от 3 + 9 (12) `

Формально, каждое значение (токен) в выражении можно считать некоторой операцией
над стеком (вершина стека слева). Вычисление операции в общем случае выглядит так:
1) она извлекается из стека
2) рекурсивно вычисляются и сохраняются необходимые операнды
3) вычисляется значение операции
4) результат помещается в стек (в виде константы)

Сумма - бинарная операция: она извлекается; вычисляем первый операнд как
операцию на вершине стека; результат на вершине - извлекаем и сохраняем;
аналогично вычисляем второй операнд; суммируем два операнда; результат
помещаем на вершину стека.

Видно, что вычисление выражения сводится к вычислению первой операции
в стеке (все остальное вычисляется рекурсивно). В случае, если
выражение было составлено верно, то в результате рекурсивных вызовов
в стеке останется только одна операция - константа, в которой
будет записан ответ.

*Пример:*
Изначально стек имеет вид `[+, 3, + 4, 5]`.

1. Извлекаем +, он требует двух операндов, которые надо будет вычислить: `+(..., ...), [3, +, 4, 5]`
2. Рекурсивно запускаем вычисление от стека для получения первого операнда
3. Извлекаем 3, это константа, поэтому помещаем обратно в стек, вычисление закончено `[3, +, 4, 5]`.
4. Возвращаемся в обработку +, извлекаем верхний элемент - получен первый операнд: `+(3, ...), [+, 4, 5]`
5. Рекурсивно запускаем вычисление от стека для получения второго операнда
6. Извлекаем +, он требует двух операндов: `+(3, ...), +(..., ...), [4, 5]`
7. Вычислили первый операнд: `+(3, ...), +(4, ...), [5]`
8. Вычислили второй операнд: `+(3, ...), +(4, 5), []`
9. Вычисляем результат и помещаем на вершину стека: `+(3, ...), [9]`
10. Возвращаемся в обработку +, извлекаем верхний элемент - получен второй операнд: `+(3, 9), []`
11. Результат записываем на вершину: `[12]`
12. Все, вызов завершился, стек содержит один элемент - результат

## Общая постановка задачи

Вам необходимо написать класс Calculator который принимает на вход математическое выражение (где число это произвольный тип T) и считает его.

Необходима поддержка следующих операторов: (, ), -(унарный), +(унарный), -, +, /, *

## Сущности которые надо реализовать

### Токены

#### AbstractToken

Это класс, от которого будут наследоваться остальные токены. Должен иметь следующие методы:

1. AbstractToken(const std::string&) - конструктор от стринги.
2. const std::string& GetStringToken() const - метод, который возвращает строковое представление токена

#### BracketToken : AbstractToken

Этот класс представляет токен скобки. Нужно поддержать следующие методы:

1. BracketToken(const std::string&)
2. bool IsOpening() const - является ли скобка открывающей

#### OperandToken<T>: AbstractToken

Этот шаблонный класс представляет токен операнда. Нужно поддержать следующие методы:

1. OperandToken(const std::string&)
2. OperandToken(const T& value)
3. const T& GetValue()

***Note*** Предполагаем что ваш калькулятор будет использовать только такие типы T, которые умеют вводиться из потока. Подсказка: используйте std:stringstream

#### OperatorToken<T value, bool IsBinary>: AbstractToken

Этот класс представляет токен оператора. Нужно поддержать следующие методы:

1. OperatorToken(const std::string&)
2. bool IsBinary() - является ли оператор бинарным
3. Calculate(Operand<T>* lhs, Operand<T>* rhs) - применяет оператор к двум операндам. Не должен компилироваться если вызывается от не бинарного оператора! (static_assert или как хотите)
4. Calculate(Operand<T>* operand) - применяет унарный оператор к операнду. Не должен компилироваться если вызывается от бинарного оператора (static_assert или как хотите)

Подсказка по реализации. По сути есть два пути (путь с кучей ифов мы отметаем):

1. Реализовать класс для каждого оператора и унаследовать их от OperatorToken
2. Использовать лямбда функции

ВНИМАНИЕ: просто миллион ифов не принимается!

### Выражение

Нужно реализовать класс ExprInPolishNotation<T>. Этот класс должен конструироваться от строки и парсить выражение на токены.

Необходимо поддержать следующие методы:

1. ExprInPolishNotation(const std::string&) - парсит выражение и превращает его в вектор токенов
2. const std::vector<AbstractToken*>& GetTokens() - возвращает вектор токенов (польская нотация)

### Calculator<T>

Собственно основной класс.

Необходимо реализовать один статический метод static T CalculateExpression(const std::string& expr).

Этот метод должен обработать строку с выражением и посчитать это выражение.

При этом добавлять дополнительные статические методы не запрещается

### Некорректные выражения

Если в какой-то момент ваш код принимает некорректное выражение, то он должен:

1. Очистить всю память за собой
2. Выкинуть исключение отнаследованное от std::exception. Например такое:

```c++
struct InvalidExpr : std::exception {
  [[nodiscard]] const char *what() const noexcept override {
    return "Invalid expression!";
  }
};
```

### Важные детали реализации

1. Каждый класс должен быть реализован в своем .hpp файле. (Мы не будем разбивать на .cpp и .hpp потому что будет сложно все это линковать в тест системе).
2. Токены могут быть разделены произвольным количеством пробелов, то есть выражение "2    + 2" это тоже самое что выражение "2+2"
3. Ваш класс должен уметь работать как с целочисленными типами, так и с double/float
