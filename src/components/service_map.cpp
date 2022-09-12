#include <algorithm>
#include <graphviz/cgraph.h>
#include <graphviz/gvc.h>

#include <QtGui/QGuiApplication>

#include <QtQml/QQmlContext>
#include <QtQuick/QSGFlatColorMaterial>
#include <QtQuick/QSGGeometry>
#include <QtQuick/QSGGeometryNode>

#include "service_map.h"
#include "span_model.h"

namespace {
struct ContextDeleter
{
    void operator()(GVC_t *ctx) const
    {
        gvFinalize(ctx);
        if (gvFreeContext(ctx) != 0) {
            qWarning() << "gvFreeContext != 0";
        }
    }
};

struct GraphDeleter
{
    void operator()(Agraph_t *graph) const
    {
        if (agclose(graph) != 0) {
            qWarning() << "agclose != 0";
        }
    }
};

using GVContextPtr = std::unique_ptr<GVC_t, ContextDeleter>;
using GVGraphPtr = std::unique_ptr<Agraph_t, GraphDeleter>;

template<typename NodeType>
void setAttribute(NodeType *node, const QString &key, const QString &value)
{
    char empty[] = "";

    auto k = key.toLatin1();
    auto v = value.toLatin1();
    agsafeset(node, k.data(), v.data(), empty);
}

} // namespace

namespace components {

const QString &ServiceMapNode::name() const
{
    return process->name;
}

QStringList ServiceMapNode::operations() const
{
    QSet<QString> data;
    for (const auto &edge : inEdges) {
        for (auto span : edge.spans) {
            data.insert(span->operationName);
        }
    }
    return data.values();
}

bool ServiceMapNode::hasEdges() const
{
    return !inEdges.isEmpty();
}

struct ServiceMapCtx
{
    ServiceMapCtx()
        : ctx(gvContext())
        , graph(agopen("service_map", Agdirected, NULL))
    {
        setGraphAttribute("label", "service map");

        setGraphAttribute("rankdir", "LR");
        setGraphAttribute("nodesep", "0.5");
        //setGraphAttribute("splines", "ortho");

        setNodeAttribute("shape", "box");
        setEdgeAttribute("minlen", "3");
    }

    ~ServiceMapCtx() { gvFreeLayout(ctx.get(), graph.get()); }

    void setNodeAttribute(const QString &name, const QString &value)
    {
        if (graph) {
            agattr(graph.get(), AGNODE, name.toLocal8Bit().data(), value.toLocal8Bit().data());
        }
    }

    void setGraphAttribute(const QString &name, const QString &value)
    {
        if (graph) {
            agattr(graph.get(), AGRAPH, name.toLocal8Bit().data(), value.toLocal8Bit().data());
        }
    }

    void setEdgeAttribute(const QString &name, const QString &value)
    {
        if (graph) {
            agattr(graph.get(), AGEDGE, name.toLocal8Bit().data(), value.toLocal8Bit().data());
        }
    }

    GVContextPtr ctx;
    GVGraphPtr graph;
};

ServiceMap::ServiceMap(QQuickItem *parent)
    : QQuickItem(parent)
    , m_ctx(std::make_unique<ServiceMapCtx>())
    , m_delegate(nullptr)
{
    setFlag(QQuickItem::ItemHasContents);
}

ServiceMap::~ServiceMap() {}

const TraceGraph &ServiceMap::getGraph() const
{
    return m_trace;
}

void ServiceMap::setGraph(const TraceGraph &data)
{
    m_trace = data;
    emit notifyGraphChanged();
    makeServiceGraph();
    if (m_delegate) {
        makeQuickNodes();
        computeLayout();
    }
}

QQmlComponent *ServiceMap::delegate() const
{
    return m_delegate;
}

void ServiceMap::setDelegate(QQmlComponent *delegate)
{
    m_delegate = delegate;
    emit notifyDelegateChanged();
    if (m_trace.data) {
        makeServiceGraph();
        makeQuickNodes();
        computeLayout();
    }
}

void ServiceMap::makeServiceGraph()
{
    if (!m_nodes.isEmpty() || !m_edges.isEmpty()) {
        resetGraph();
    }

    using EdgeIndex = std::tuple<graph::Process *, graph::Process *>;

    for (int i = 0; i < m_trace.data->traces.size(); ++i) {
        const auto &trace = m_trace.data->traces[i];

        QHash<graph::Process *, int> processMap;
        processMap.reserve(trace->process.size());

        for (int n = 0; n < trace->process.size(); ++n) {
            ServiceMapNode node;
            node.process = trace->process[n].get();
            m_nodes.emplace_back(node);
            processMap.insert(trace->process[n].get(), m_nodes.size() - 1);
        }

        QMap<EdgeIndex, int> edgesMap;
        for (int s = 0; s < trace->spans.size(); ++s) {
            const auto &span = trace->spans[s];
            if (span->parent == nullptr) {
                continue;
            }

            if (span->parent->process == span->process) {
                continue;
            }

            auto indx = std::make_tuple(span->parent->process, span->process);
            if (edgesMap.contains(indx)) {
                m_edges[edgesMap[indx]].spans.push_back(span.get());
            } else {
                ServiceMapEdge edge;
                edge.from = span->parent->process;
                edge.to = span->process;
                edge.spans.push_back(span.get());
                m_edges.emplace_back(edge);
                edgesMap[indx] = m_edges.size() - 1;
            }
        }

        for (const auto &edge : m_edges) {
            auto indx = processMap[edge.to];
            m_nodes[indx].inEdges.push_back(edge);
        }
    };
}

void ServiceMap::resetGraph()
{
    for (auto node : m_nodes) {
        if (node.qmlObject) {
            node.qmlObject->deleteLater();
        }
    }
    m_nodes.clear();

    for (auto edge : m_edges) {
        if (edge.qmlObject) {
            edge.qmlObject->deleteLater();
        }
    }
    m_edges.clear();
    m_ctx.release();
    m_ctx = std::make_unique<ServiceMapCtx>();
}

void ServiceMap::makeQuickNodes()
{
    for (auto &node : m_nodes) {
        auto creationCtx = m_delegate->creationContext();
        auto ctx = new QQmlContext(creationCtx ? creationCtx : qmlContext(this));

        auto item = m_delegate->beginCreate(ctx);
        if (item) {
            ctx->setContextProperty("node", QVariant::fromValue(node));

            auto quickItem = qobject_cast<QQuickItem *>(item);

            quickItem->setParentItem(this);
            quickItem->setZ(1);
            node.qmlObject = quickItem;
        } else {
            qCritical() << "failed create QQuickItem from delegate" << m_delegate->errors();
        }
        m_delegate->completeCreate();
    }

    for (auto &edge : m_edges) {
        edge.qmlObject = new EdgeItem;
        edge.qmlObject->setZ(2);
        edge.qmlObject->setParentItem(this);
    }
}

void applySize(ServiceMapNode &node, qreal DPI)
{
    if (node.qmlObject) {
        auto size = node.qmlObject->size();
        auto widthIn = QString::number(qreal(size.width()) / DPI);
        auto heightIn = QString::number(qreal(size.height()) / DPI);
        setAttribute(node.gvNode, "width", widthIn);
        setAttribute(node.gvNode, "height", heightIn);
        setAttribute(node.gvNode, "fixedsize", "true");
    }
}

QPointF centerToOrigin(const QPointF &p, qreal width, qreal height)
{
    return QPointF(p.x() - width / 2, p.y() - height / 2);
}

void ServiceMap::computeLayout()
{
    auto graph = m_ctx->graph.get();
    QHash<graph::Process *, Agnode_t *> nodeMap;
    for (auto &node : m_nodes) {
        node.gvNode = agnode(graph, NULL, true);
        nodeMap.insert(node.process, node.gvNode);
        applySize(node, DPI);
    }

    for (auto &edge : m_edges) {
        auto from = nodeMap[edge.from];
        auto to = nodeMap[edge.to];
        edge.gvEdge = agedge(graph, from, to, NULL, TRUE);
    }

    if (gvLayout(m_ctx->ctx.get(), graph, "dot") != 0) {
        qCritical() << "Layout render error" << agerrors() << QString::fromLocal8Bit(aglasterr());
    }

    qreal gvGraphHeight = GD_bb(graph).UR.y;
    qreal gvGraphWidth = GD_bb(graph).UR.x;
    setImplicitHeight(gvGraphHeight);
    setImplicitWidth(gvGraphWidth);

    for (auto &node : m_nodes) {
        auto gvPos = ND_coord(node.gvNode);
        QPoint pt(gvPos.x, gvGraphHeight - gvPos.y);
        auto org = centerToOrigin(pt, node.qmlObject->width(), node.qmlObject->height());
        node.qmlObject->setPosition(org);
    }

    for (auto &edge : m_edges) {
        auto spline = ED_spl(edge.gvEdge);
        QVector<QPointF> points;

        if (spline->size != 0) {
            bezier bez = spline->list[0];
            points.reserve(bez.size);

            for (int i = 0; i < bez.size; ++i) {
                auto &p = bez.list[i];
                points << QPointF(p.x, gvGraphHeight - p.y);
            }
            points << QPointF(spline->list->ep.x, gvGraphHeight - spline->list->ep.y);
        }

        edge.qmlObject->setPoints(points);
    }
    update();
}

EdgeItem::EdgeItem(QQuickItem *parent)
    : QQuickItem(parent)
    , m_arrowNode(nullptr)
{
    setFlag(ItemHasContents);
}

QSGNode *EdgeItem::updatePaintNode(QSGNode *oldNode, UpdatePaintNodeData *)
{
    if (!m_arrowNode) {
        m_arrowNode = new QSGGeometryNode;
        auto geometry = new QSGGeometry(QSGGeometry::defaultAttributes_Point2D(), 3);
        geometry->setLineWidth(1);
        geometry->setDrawingMode(QSGGeometry::DrawTriangles);
        m_arrowNode->setGeometry(geometry);
        m_arrowNode->setFlag(QSGNode::OwnsGeometry);
        auto *material = new QSGFlatColorMaterial;
        material->setColor(QColor("black"));
        m_arrowNode->setMaterial(material);
        m_arrowNode->setFlag(QSGNode::OwnsMaterial);
        geometry->allocate(3);
    }

    QSGGeometryNode *node = nullptr;
    QSGGeometry *geometry = nullptr;

    if (!oldNode) {
        node = new QSGGeometryNode;
        geometry = new QSGGeometry(QSGGeometry::defaultAttributes_Point2D(),
                                   std::max(m_points.size() - 1, qsizetype(0)));
        geometry->setLineWidth(1);
        geometry->setDrawingMode(QSGGeometry::DrawLineStrip);
        node->setGeometry(geometry);
        node->setFlag(QSGNode::OwnsGeometry);
        auto *material = new QSGFlatColorMaterial;
        material->setColor(QColor("black"));
        node->setMaterial(material);
        node->setFlag(QSGNode::OwnsMaterial);

        node->appendChildNode(m_arrowNode);
    } else {
        node = static_cast<QSGGeometryNode *>(oldNode);
        geometry = node->geometry();
        geometry->allocate(m_points.size() - 1);
    }

    QSGGeometry::Point2D *vertices = geometry->vertexDataAsPoint2D();

    for (int i = 0; i < m_points.size() - 1; ++i) {
        vertices[i].set(m_points[i].x(), m_points[i].y());
    }

    if (!m_points.isEmpty()) {
        QLineF line(m_points[m_points.size() - 2], m_points[m_points.size() - 1]);
        QLineF n = line.normalVector();
        QPointF o(n.dx() / 3.0, n.dy() / 3.0);

        auto arrVertices = m_arrowNode->geometry()->vertexDataAsPoint2D();
        arrVertices[0].set(line.p1().x() + o.x(), line.p1().y() + o.y());
        arrVertices[1].set(line.p2().x(), line.p2().y());
        arrVertices[2].set(line.p1().x() - o.x(), line.p1().y() - o.y());
    }

    node->markDirty(QSGNode::DirtyGeometry);
    return node;
}

void EdgeItem::setPoints(const QVector<QPointF> &points)
{
    m_points = points;
    update();
}

ServiceMapNodeItem::ServiceMapNodeItem(QObject *parent)
    : QObject(parent)
    , m_process(nullptr)
    , m_spanModel(new SpanModel(this))
    , m_tagModel(new TagModel(this))
{}

void ServiceMapNodeItem::setNode(const ServiceMapNode &node)
{
    m_process = node.process;
    notify();
    QVector<graph::Span *> spans;
    for (const auto &edge : node.inEdges) {
        spans.append(edge.spans);
    }

    m_spanModel->setSpans(std::move(spans));
    m_tagModel->setTags(node.process->tags);
}

QString ServiceMapNodeItem::processName() const
{
    if (m_process) {
        return m_process->name;
    }

    return {};
}

SpanModel *ServiceMapNodeItem::spanModel() const
{
    return m_spanModel;
}

TagModel *ServiceMapNodeItem::tagModel() const
{
    return m_tagModel;
}

void ServiceMapNodeItem::notify()
{
    emit notifyProcessNameChanged();
}
} // namespace components
