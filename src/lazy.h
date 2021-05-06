#pragma once
#include <QMap>
#include <QtMath>

#define OPERATOR(name)                                                                                                 \
    Lazy *name(Lazy *);                                                                                                \
    static inline Lazy *name(Lazy *l1, Lazy *l2)                                                                       \
    {                                                                                                                  \
        return l1->name(l2);                                                                                           \
    }

class Lazy
{
public:
    Lazy();
    virtual qreal value() = 0;

    OPERATOR(sum);
    OPERATOR(sub);
    OPERATOR(mul);
    OPERATOR(div);

    Lazy *rec();
    Lazy *neg();

    static Lazy *number(qreal x);
    static Lazy *number(qreal *x);
    static Lazy *sin(Lazy *l);
    static Lazy *cos(Lazy *l);
    static Lazy *abs(Lazy *l);

    static Lazy *fromString(QByteArray str, const QMap<QByteArray, Lazy *> variables);
};

inline Lazy::Lazy()
{
}

class Number : public Lazy
{
public:
    Number(qreal x) : x(x), x_p(&(this->x)){};
    Number(qreal *x_p) : x_p(x_p){};
    qreal value()
    {
        return *x_p;
    };

private:
    qreal x;
    qreal *x_p;
};

class Rec : public Lazy
{
public:
    Rec(Lazy *x) : x(x)
    {
    }
    qreal value()
    {
        return 1 / x->value();
    }

private:
    Lazy *x;
};

class Sum : public Lazy
{
public:
    Sum(Lazy *l1, Lazy *l2) : l1(l1), l2(l2){};
    qreal value()
    {
        return l1->value() + l2->value();
    };

private:
    Lazy *l1, *l2;
};

class Mul : public Lazy
{
public:
    Mul(Lazy *l1, Lazy *l2) : l1(l1), l2(l2){};
    qreal value()
    {
        return l1->value() * l2->value();
    };

private:
    Lazy *l1, *l2;
};

#define LazyFunc(name, func)                                                                                           \
    class name : public Lazy                                                                                           \
    {                                                                                                                  \
    public:                                                                                                            \
        name(Lazy *l) : l(l){};                                                                                        \
        qreal value()                                                                                                  \
        {                                                                                                              \
            return func(l->value());                                                                                   \
        };                                                                                                             \
                                                                                                                       \
    private:                                                                                                           \
        Lazy *l;                                                                                                       \
    };

LazyFunc(Sin, qSin);
LazyFunc(Cos, qCos);
LazyFunc(Abs, qAbs);
