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

Lazy *Lazy::fromString(QByteArray str, const QMap<QByteArray, Lazy *> &variables)
{
    str.replace(QChar::Space, "");
    return fromStringPrivate(str, variables);
}

using operator_priority = quint16;

int operatorOfExp(const QString &str, const QMap<QChar, operator_priority> &operators_priorities)
{
    int operator_pos = 0;
    int open_parens = 0;
    if (str[0] == '(')
    {
        ++open_parens;
    }
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
        else if (operators_priorities.contains(str[i]) && open_parens == 0 &&
                 (operators_priorities[str[i]] < operators_priorities[str[operator_pos]] || operator_pos == 0))
        {
            operator_pos = i;
        }
    }
    return operator_pos;
}

Lazy *Lazy::fromStringPrivate(const QByteArray &str, const QMap<QByteArray, Lazy *> variables)
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

    bool is_number = false;
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
        return OPERATORS[(QChar)str[operator_pos]](fromStringPrivate(str.left(operator_pos), variables),
                                                   fromStringPrivate(str.mid(operator_pos + 1), variables));
    }

    if (str[0] == '-')
    {
        return Lazy::fromStringPrivate(str.mid(1), variables)->neg();
    }

    if (str.contains('('))
    {
        if (str[0] == '(')
        {
            return fromStringPrivate(str.mid(1).chopped(1), variables);
        }
        int paren_pos = str.indexOf('(');
        return FUNCTIONS[str.left(paren_pos)](fromStringPrivate(str.mid(paren_pos + 1).chopped(1), variables));
    }

    qCritical() << "Cannot initialize `Lazy` from" << str;
}

Lazy *Lazy::rec()
{
    return new Rec(this);
}
