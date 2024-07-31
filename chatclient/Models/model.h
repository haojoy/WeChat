#ifndef MODEL_H
#define MODEL_H
#include <QObject>

/**
 * @brief The Model class 自定义元类型
 */
class Model
{
public:
    Model();
};
Q_DECLARE_METATYPE(Model)
Q_DECLARE_METATYPE(Model*)
#endif // MODEL_H
