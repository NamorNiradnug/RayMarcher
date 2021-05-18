#include "scene.h"

Lazy *Scene::addVariable(const QByteArray &name, Lazy *value)
{
    variables.insert(name, value);
    return value;
}

Lazy *Scene::variable(const QByteArray &varname) const
{
    return variables[varname];
}

Scene::Scene(const QString &f) : f(f){};

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
        is_prev_backslash = (ch == '\\' && !is_prev_backslash);
    }
    splitted.append(cur);

    return splitted;
}

void Scene::parse(const QSet<const QByteArray> &cmds)
{
    QFile f(this->f);
    f.open(QFile::ReadOnly);

    while (!f.atEnd())
    {
        QByteArray line = "\\";
        while (!line.isEmpty() && line.back() == '\\')
        {
            line.chop(1);
            line += f.readLine();
            if (line.contains(COMMENT_SEQ))
            {
                line = line.left(line.indexOf(COMMENT_SEQ));
            }
            line = line.simplified();
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
                const QByteArray new_var_type = vartype + vartypes[params["o1"]] + vartypes[params["o2"]];
                if (!vartypes.values().contains(new_var_type))
                {
                    macros[SDFTYPES_MACRO].append(vartype.toUpper() + "(" + vartypes[params["o1"]] + ", " +
                                                  vartypes[params["o2"]] +
                                                  ");\n"); // creates template type as INTERSECTION(...)
                }
                vartypes.insert(varname, new_var_type);
                macros[UNIFORMS_MACRO].append("uniform Transform " + GENERATED_PREFIX + TRANSFORM_PREFIX + varname +
                                              ";\n");
                macros[SDSCENE_MACRO].append(vartypes[varname] + " " + GENERATED_PREFIX + varname + " = " +
                                             vartypes[varname] + "(" + GENERATED_PREFIX + params["o1"] + ", " +
                                             GENERATED_PREFIX + params["o2"] + ", ");
                for (const auto &p : TEMPLATES_TYPES[vartype])
                {
                    macros[SDSCENE_MACRO] += params.value(p, "0.0") + ", ";
                }
                macros[SDSCENE_MACRO].append(GENERATED_PREFIX + TRANSFORM_PREFIX + varname + ");\n");
                transform_uniform_name = TRANSFORM_PREFIX + varname;
            }
            else
            {
                vartypes.insert(varname, vartype);
                macros[UNIFORMS_MACRO].append("uniform " + vartypes[varname] + " " + GENERATED_PREFIX + varname +
                                              ";\n");
                for (const QByteArray &param : params.keys())
                {
                    params_values.insert(GENERATED_PREFIX + varname + "." + param,
                                         Lazy::fromString(params[param], variables));
                }
                transform_uniform_name = varname + ".t";
            }
            rotations.insert(
                GENERATED_PREFIX + transform_uniform_name + ".rotation",
                {Lazy::fromString(words[read_words++], variables), Lazy::fromString(words[read_words++], variables),
                 Lazy::fromString(words[read_words++], variables), Lazy::fromString(words[read_words++], variables)});
            params3d_values.insert(GENERATED_PREFIX + transform_uniform_name + ".translation",
                                   {Lazy::fromString(words[read_words++], variables),
                                    Lazy::fromString(words[read_words++], variables),
                                    Lazy::fromString(words[read_words++], variables)});
            params_values.insert(GENERATED_PREFIX + transform_uniform_name + ".scale",
                                 Lazy::fromString(words[read_words++], variables));
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
            params3d_values.insert("sun.color",
                                   {Lazy::fromString(words[1], variables), Lazy::fromString(words[2], variables),
                                    Lazy::fromString(words[3], variables)});
        }
        else if (CHECK_CMD(CMD_SDFTYPE))
        {
            macros[SDFTYPES_MACRO].append("SDTYPE(" + words[1] + ",\n{\n");
            int pos = 2;
            for (; words[pos][0] != '{'; ++pos)
            {
                macros[SDFTYPES_MACRO].append("    float " + words[pos] + ";\n");
            }
            macros[SDFTYPES_MACRO].append("    Transform t;\n},\n" + words[pos] + "\n);\n");
        }
        else if (CHECK_CMD(CMD_SUN))
        {
            normal_params3d_values.insert("sun.dir",
                                          {Lazy::fromString(words[1], variables), Lazy::fromString(words[2], variables),
                                           Lazy::fromString(words[3], variables)});
        }
    };

#undef CHECK_CMD

    macros[SDSCENE_MACRO] += "return sdist(p, " + GENERATED_PREFIX + scene_var + ");\n";
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

void Scene::setUniformsValues(QOpenGLShaderProgram *prog) const
{
    for (const auto &param : params_values.keys())
    {
        prog->setUniformValue(param.constData(), (GLfloat)params_values[param]->value());
    }
    for (const auto &rot : rotations.keys())
    {
        prog->setUniformValue(rot.constData(), rotations[rot].value());
    }
    for (const auto &tr : params3d_values.keys())
    {
        prog->setUniformValue(tr.constData(), params3d_values[tr].value());
    }
    for (const auto &tr : normal_params3d_values.keys())
    {
        prog->setUniformValue(tr.constData(), normal_params3d_values[tr].value().normalized());
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
