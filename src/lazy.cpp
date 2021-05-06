#include "lazy.h"

#include <QDebug>
#include <QMap>
#include <QRegularExpression>
#include <QString>

Lazy *Lazy::sum(Lazy *l)
{
    return new Sum(this, l);
}

Lazy *Lazy::sub(Lazy *l)
{
    return new Sum(this, l->neg());
}

Lazy *Lazy::mul(Lazy *l)
{
    return new Mul(this, l);
}

Lazy *Lazy::div(Lazy *l)
{
    return new Mul(this, l->rec());
}

Lazy *Lazy::neg()
{
    static Lazy *M_ONE = new Number(-1.0);
    return new Mul(this, M_ONE);
}

Lazy *Lazy::number(qreal x)
{
    return new Number(x);
}

Lazy *Lazy::number(qreal *x)
{
    return new Number(x);
}

Lazy *Lazy::sin(Lazy *l)
{
    return new Sin(l);
}

Lazy *Lazy::cos(Lazy *l)
{
    return new Cos(l);
}

Lazy *Lazy::abs(Lazy *l)
{
    return new Abs(l);
}

typedef quint16 operator_priority;

int operatorOfExp(const QString &str, const QMap<QChar, operator_priority> &operators)
{
    int operator_pos = 0;
    int open_parens = 0;
    for (int i = 1; i < str.size(); ++i)
    {
        if (str[i] == '(')
        {
            ++open_parens;
        }
        else if (str[i] == ')')
        {
            --open_parens;
        }
        else if (operators.contains(str[i]) && open_parens == 0 &&
                 (operators[str[i]] < operators[str[operator_pos]] || operator_pos == 0))
        {
            operator_pos = i;
        }
    }
    return operator_pos;
}

Lazy *Lazy::fromString(QByteArray str, const QMap<QByteArray, Lazy *> variables)
{
    static const QMap<QChar, Lazy *(*)(Lazy *, Lazy *)> OPERATORS = {
        {'+', &Lazy::sum},
        {'-', &Lazy::sub},
        {'*', &Lazy::mul},
        {'/', &Lazy::div},
    };
    static const QMap<QChar, operator_priority> PRIORITES = {
        {'+', 1},
        {'-', 1},
        {'*', 2},
        {'/', 3},
    };
    static const QMap<QByteArray, Lazy *(*)(Lazy *)> FUNCTIONS = {
        {"cos", &Lazy::cos},
        {"sin", &Lazy::sin},
        {"abs", &Lazy::abs},
    };
    static const QList<QChar> PUNCTUATION_CHARS = OPERATORS.keys() + QList<QChar>({'(', ')'});

    // removing useless parentheses around the expression
    int starting_parens_numer = 0;
    while (str[starting_parens_numer] == '(')
    {
        ++starting_parens_numer;
    }
    str.chop(starting_parens_numer);
    str.remove(0, starting_parens_numer);

    bool is_number;
    qreal number = str.toDouble(&is_number);
    if (is_number)
    {
        return Lazy::number(number);
    }

    if (variables.contains(str))
    {
        return variables[str];
    }

    int operator_pos = operatorOfExp(str, PRIORITES);
    if (operator_pos != 0)
    {
        return OPERATORS[(QChar)str[operator_pos]](Lazy::fromString(str.left(operator_pos), variables),
                                                   Lazy::fromString(str.mid(operator_pos + 1), variables));
    }

    if (str[0] == '-')
    {
        return Lazy::fromString(str.mid(1), variables)->neg();
    }

    if (str.contains('('))
    {
        int paren_pos = str.indexOf('(');
        return FUNCTIONS[str.left(paren_pos)](Lazy::fromString(str.mid(paren_pos + 1).chopped(1), variables));
    }

    qCritical() << "Cannot initialize `Lazy` from" << str;
}

Lazy *Lazy::rec()
{
    return new Rec(this);
}
