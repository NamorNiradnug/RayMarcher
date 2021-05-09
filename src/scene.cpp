#include "scene.h"

#include <QStack>

Lazy *Scene::addVariable(const QByteArray &name, Lazy *value)
{
    variables.insert(name, value);
    return value;
}

Lazy *Scene::variable(const QByteArray &varname) const
{
    return variables[varname];
}

Scene::Scene(QString f) : f(f){};

QByteArrayList split(const QByteArray &line)
{
    QByteArrayList splitted;
    QByteArray cur;
    int open_paren = 0;
    QChar opening_quote = QChar::Space;
    bool is_prev_backslash = false;
    for (const auto &ch : line)
    {
        if (ch == QChar::Space && open_paren == 0 && opening_quote == QChar::Space)
        {
            cur.replace(QChar::Space, "");
            splitted.append(cur);
            cur.clear();
        }
        else if (ch == '"' || ch == '\'')
        {
            if (cur.isEmpty())
            {
                opening_quote = ch;
            }
            else if (!is_prev_backslash && ch == opening_quote)
            {
                opening_quote = QChar::Space;
            }
            else
            {
                cur.append(ch);
            }
        }
        else
        {
            if (ch == '(')
            {
                ++open_paren;
            }
            else if (ch == ')')
            {
                open_paren = qMax(--open_paren, 0);
            }
            if (ch != '\\' || is_prev_backslash)
            {
                cur.append(ch);
            }
        }
        if (ch == '\\' && !is_prev_backslash)
        {
            is_prev_backslash = true;
        }
        else
        {
            is_prev_backslash = false;
        }
    }
    splitted.append(cur);

    return splitted;
}

void Scene::parse(const QSet<const QByteArray> &cmds)
{
    QByteArray before_uniforms;
    QByteArray uniforms;
    QByteArray sdscene;
    QByteArray color;
    QFile f(this->f);
    f.open(QFile::ReadOnly);

    while (!f.atEnd())
    {
        QByteArray line = f.readLine().simplified();
        if (line.contains(COMMENT_SEQ))
        {
            line = line.left(line.indexOf(COMMENT_SEQ));
        }
        const QByteArrayList words = split(line);
        if (words.empty())
        {
            continue;
        }

        const QByteArray cmd = words[0];

#define CHECK_CMD(CMD) (cmd == (CMD) && (cmds.isEmpty() || cmds.contains(CMD)))

        if (CHECK_CMD(CMD_VAR_DEF))
        {
            /// SDF $VARTYPE $VARNAME $p1=$exp1 $p2=$ext2 ... $transform
            const QByteArray varname = words[1];
            const QByteArray vartype = words[2];
            QMap<QByteArray, QByteArray> params;
            int read_words = 3;
            for (; words[read_words].contains('='); ++read_words)
            {
                params.insert(words[read_words].split('=')[0], words[read_words].split('=')[1]);
            }

            QByteArray transform_uniform_name;
            if (TEMPLATES_TYPES.contains(vartype))
            {
                if (!vartypes.values().contains(vartypes[varname]))
                {
                    before_uniforms.append(vartype.toUpper() + "(" + vartypes[params["o1"]] + ", " +
                                           vartypes[params["o2"]] +
                                           ");\n"); // creates template type as INTERSECTION(...)
                }
                vartypes.insert(varname, vartype + vartypes[params["o1"]] + vartypes[params["o2"]]);
                uniforms.append("uniform Transform " + TRANSFORM_PREFIX + varname + ";\n");
                sdscene.append(vartypes[varname] + " " + varname + " = " + vartypes[varname] + "(" + params["o1"] +
                               ", " + params["o2"] + ", " + TRANSFORM_PREFIX + varname + ");\n");
                transform_uniform_name = TRANSFORM_PREFIX + varname;
            }
            else
            {
                vartypes.insert(varname, vartype);
                uniforms.append("uniform " + vartypes[varname] + " " + varname + ";\n");
                for (const QByteArray &param : params.keys())
                {
                    params_values.insert(varname + "." + param, Lazy::fromString(params[param], variables));
                }
                transform_uniform_name = varname + ".t";
            }

            rotations.insert(transform_uniform_name + ".rotation", {Lazy::fromString(words[read_words++], variables),
                                                                    Lazy::fromString(words[read_words++], variables),
                                                                    Lazy::fromString(words[read_words++], variables),
                                                                    Lazy::fromString(words[read_words++], variables)});
            translations.insert(transform_uniform_name + ".translation",
                                {Lazy::fromString(words[read_words++], variables),
                                 Lazy::fromString(words[read_words++], variables),
                                 Lazy::fromString(words[read_words++], variables)});
            params_values.insert(transform_uniform_name + ".scale", Lazy::fromString(words[read_words++], variables));
        }
        else if (CHECK_CMD(CMD_SCENE_DEF))
        {
            scene_var = words[1];
        }
        else if (CHECK_CMD(CMD_NAME))
        {
            name = words[1];
        }
        else if (CHECK_CMD(CMD_COLOR_DEF))
        {
            macros[COLOR_MACRO] = words[1];
        }
    };
    macros[UNIFROMS_MACRO] = before_uniforms + uniforms;
    macros[SDSCENE_MACRO] = sdscene + "return sdist(p, " + scene_var + ");\n";
    for (const auto &macro : macros.keys())
    {
        macros[macro].replace("\n", " \\\n");
    }
    f.close();
    if (cmds.isEmpty())
    {
        parsed = true;
    }
}

QByteArray Scene::generateShader() const
{
    QFile f(TEMPLATE_SHADER_PATH);
    f.open(QFile::ReadOnly);
    QByteArray sources = f.readLine();
    for (const auto &macro : macros.keys())
    {
        sources += "#define " + macro + " \\\n" + macros[macro] + "\n";
    }
    sources += f.readAll();
    f.close();
    return sources;
}

QVector4D rotation(RotationSource r)
{
    return QVector4D(QVector3D(r.x->value(), r.y->value(), r.z->value()).normalized() * qSin(r.angle->value() / 2),
                     qCos(r.angle->value() / 2));
}

inline QVector3D vec3(LazyVector3D l3)
{
    return QVector3D(l3.x->value(), l3.y->value(), l3.z->value());
}

void Scene::setUniformsValues(QOpenGLShaderProgram *prog) const
{
    for (const auto &param : params_values.keys())
    {
        prog->setUniformValue(param.constData(), (GLfloat)params_values[param]->value());
    }
    for (const auto &rot : rotations.keys())
    {
        prog->setUniformValue(rot.constData(), rotation(rotations[rot]));
    }
    for (const auto &tr : translations.keys())
    {
        prog->setUniformValue(tr.constData(), vec3(translations[tr]));
    }
}

const QByteArray &Scene::getName()
{
    if (parsed)
    {
        return name;
    }
    parse({CMD_NAME});
    return name;
}

bool Scene::isParsed() const
{
    return parsed;
}

const QString &Scene::getFile() const
{
    return f;
}
