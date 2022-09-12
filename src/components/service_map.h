#pragma once

#include <QtQuick/QQuickItem>
#include <QtQuick/QSGGeometryNode>

#include <graphviz/cgraph.h>

#include "span_model.h"
#include "tag_model.h"
#include "trace.h"

namespace components {

struct ServiceMapEdge;
class EdgeItem;

struct ServiceMapNode
{
    Q_GADGET
    Q_PROPERTY(QString name READ name)
    Q_PROPERTY(QStringList operations READ operations)
    Q_PROPERTY(bool hasEdges READ hasEdges)

public:
    QVector<ServiceMapEdge> inEdges;
    graph::Process *process = nullptr;
    QQuickItem *qmlObject = nullptr;
    Agnode_t *gvNode = nullptr;

public:
    const QString &name() const;
    QStringList operations() const;
    bool hasEdges() const;
};

struct ServiceMapEdge
{
    Q_GADGET
public:
    graph::Process *from = nullptr;
    graph::Process *to = nullptr;
    QVector<graph::Span *> spans;
    EdgeItem *qmlObject = nullptr;
    Agedge_t *gvEdge = nullptr;
};

class EdgeItem : public QQuickItem
{
    Q_OBJECT
    QML_ELEMENT
public:
    explicit EdgeItem(QQuickItem *parent = nullptr);
    QSGNode *updatePaintNode(QSGNode *, UpdatePaintNodeData *) override;

    void setPoints(const QVector<QPointF> &points);

private:
    QVector<QPointF> m_points;
    QSGGeometryNode *m_arrowNode;
};

struct ServiceMapCtx;

class ServiceMap : public QQuickItem
{
    Q_OBJECT
    Q_PROPERTY(TraceGraph graph READ getGraph WRITE setGraph NOTIFY notifyGraphChanged)
    Q_PROPERTY(QQmlComponent *delegate READ delegate WRITE setDelegate NOTIFY notifyDelegateChanged)
public:
    static constexpr qreal DPI = 72.0; //https://graphviz.org/doc/info/attrs.html

    explicit ServiceMap(QQuickItem *parent = nullptr);
    ~ServiceMap();

    const TraceGraph &getGraph() const;
    void setGraph(const TraceGraph &data);

    QQmlComponent *delegate() const;
    void setDelegate(QQmlComponent *delegate);

signals:

    void notifyGraphChanged();
    void notifyDelegateChanged();

private:
    void makeServiceGraph();
    void makeQuickNodes();
    void computeLayout();
    void resetGraph();

private:
    std::unique_ptr<ServiceMapCtx> m_ctx;
    TraceGraph m_trace;
    QQmlComponent *m_delegate;

    QVector<ServiceMapNode> m_nodes;
    QVector<ServiceMapEdge> m_edges;
};

class ServiceMapNodeItem : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString processName READ processName NOTIFY notifyProcessNameChanged)
    Q_PROPERTY(SpanModel *spanModel READ spanModel NOTIFY notifySpanModelChanged)
    Q_PROPERTY(TagModel *tagModel READ tagModel NOTIFY notifyTagModelChanged)

public:
    explicit ServiceMapNodeItem(QObject *parent = nullptr);
    Q_INVOKABLE void setNode(const ServiceMapNode &node);

    QString processName() const;
    SpanModel *spanModel() const;
    TagModel *tagModel() const;

signals:

    void notifyProcessNameChanged();
    void notifySpanModelChanged();
    void notifyTagModelChanged();

private:
    void notify();

private:
    graph::Process *m_process;
    SpanModel *m_spanModel;
    TagModel *m_tagModel;
};
} // namespace components

Q_DECLARE_METATYPE(components::ServiceMapNode)