#include "dot_graph/dot_graph.h"

#include <sstream>
#include <string>
#include <utility>

namespace dot_graph {

namespace {

inline void
indent( std::ostream& os, size_t width )
{
    for ( size_t i = 0; i < width; ++i )
    {
        os << ' ';
    }
}

} // namespace

void
Attributes::Entry::write( std::ostream& os, size_t width ) const
{
    indent( os, width );

    os << key << '=';

    switch ( kind )
    {
        case Attributes::ValueKind::QUOTED:
            os << '"' << value << '"';
            break;
        case Attributes::ValueKind::RAW:
            os << value;
            break;
        case Attributes::ValueKind::HTML:
            os << '<' << value << '>';
            break;
    }
}

Attributes::Entry&
Attributes::upsert( std::string_view key )
{
    for ( auto& entry : entries_ )
    {
        if ( entry.key == key )
        {
            return entry;
        }
    }

    entries_.push_back( Entry{ std::string( key ) } );
    return entries_.back();
}

Attributes&
Attributes::set( std::string_view key, std::string_view value, ValueKind kind )
{
    auto& entry = upsert( key );
    entry.value = std::string( value );
    entry.kind  = kind;
    return *this;
}

Attributes&
Attributes::setQuoted( std::string_view key, std::string_view value )
{
    return set( key, value, ValueKind::QUOTED );
}

Attributes&
Attributes::setRaw( std::string_view key, std::string_view value )
{
    return set( key, value, ValueKind::RAW );
}

Attributes&
Attributes::setHtml( std::string_view key, std::string_view value )
{
    return set( key, value, ValueKind::HTML );
}

template<typename TNum>
Attributes&
Attributes::setNumber( std::string_view key, TNum num )
{
    auto& entry = upsert( key );
    entry.value = std::to_string( num );
    entry.kind  = ValueKind::RAW;
    return *this;
}

Attributes&
Attributes::setBool( std::string_view key, bool value )
{
    auto& entry = upsert( key );
    entry.value = value ? "true" : "false";
    entry.kind  = ValueKind::RAW;
    return *this;
}

Attributes&
Attributes::setQuotedLabel( std::string_view value )
{
    return setQuoted( "label", value );
}

Attributes&
Attributes::setHtmlLabel( std::string_view value )
{
    return setHtml( "label", value );
}

Attributes&
Attributes::setColor( std::string_view value )
{
    return setQuoted( "color", value );
}

Attributes&
Attributes::setFillColor( std::string_view value )
{
    return setQuoted( "fillcolor", value );
}

Attributes&
Attributes::setFontColor( std::string_view value )
{
    return setQuoted( "fontcolor", value );
}

Attributes&
Attributes::setFontName( std::string_view value )
{
    return setQuoted( "fontname", value );
}

Attributes&
Attributes::setFontSize( std::size_t value )
{
    return setNumber( "fontsize", value );
}

Attributes&
Attributes::setShape( std::string_view value )
{
    return setQuoted( "shape", value );
}

Attributes&
Attributes::setStyle( std::string_view value )
{
    return setQuoted( "style", value );
}

Attributes&
Attributes::setPenWidth( std::size_t value )
{
    return setNumber( "penwidth", value );
}

Attributes&
Attributes::setWeight( std::size_t value )
{
    return setNumber( "weight", value );
}

Attributes&
Attributes::setConstraint( bool value )
{
    return setBool( "constraint", value );
}

bool
Attributes::empty() const
{
    return entries_.empty();
}

const std::vector<Attributes::Entry>&
Attributes::entries() const
{
    return entries_;
}

void
Attributes::write( std::ostream& os, size_t width ) const
{
    indent( os, width );

    os << '[';

    bool first = true;
    for ( const auto& entry : entries_ )
    {
        if ( !first )
        {
            os << ", ";
        }
        first = false;

        entry.write( os );
    }

    os << ']';
}

Node::Node( std::string id ) : id_( std::move( id ) ) {}

const std::string&
Node::id() const
{
    return id_;
}

void
Node::write( std::ostream& os, size_t width ) const
{
    indent( os, width );

    os << id_;
    if ( !attributes_.empty() )
    {
        os << ' ';
        attributes_.write( os );
    }
    os << ";\n";
}

Edge::Edge( std::string from, std::string to )
    : from_( std::move( from ) ), to_( std::move( to ) )
{
}

const std::string&
Edge::from() const
{
    return from_;
}

const std::string&
Edge::to() const
{
    return to_;
}

void
Edge::write( std::ostream& os, [[maybe_unused]] size_t width ) const
{
    os << from_ << " -> " << to_;
    if ( !attributes_.empty() )
    {
        os << ' ';
        attributes_.write( os );
    }
    os << ";\n";
}

Subgraph::Subgraph( std::string id ) : id_( std::move( id ) ) {}

const std::string&
Subgraph::id() const
{
    return id_;
}

Subgraph&
Subgraph::addSubgraph( std::string id )
{
    subgraphs_.push_back( std::make_unique<Subgraph>( std::move( id ) ) );
    return *subgraphs_.back();
}

Node&
Subgraph::addNode( std::string id )
{
    nodes_.push_back( std::make_unique<Node>( std::move( id ) ) );
    return *nodes_.back();
}

const std::vector<std::unique_ptr<Subgraph>>&
Subgraph::subgraphs() const
{
    return subgraphs_;
}

const std::vector<std::unique_ptr<Node>>&
Subgraph::nodes() const
{
    return nodes_;
}

void
Subgraph::write( std::ostream& os, size_t width ) const
{
    indent( os, width );
    os << "subgraph " << id_ << " {\n";

    for ( const auto& entry : attributes_.entries() )
    {
        entry.write( os, width + 2 );
        os << ";\n";
    }

    for ( const auto& nested_subgraph : subgraphs_ )
    {
        nested_subgraph->write( os, width + 2 );
    }

    for ( const auto& node : nodes_ )
    {
        node->write( os, width + 2 );
    }

    indent( os, width );
    os << "}\n";
}

Graph::Graph( std::string id ) : id_( std::move( id ) ) {}

Attributes&
Graph::graphAttributes()
{
    return graph_attributes_;
}

Attributes&
Graph::nodeAttributes()
{
    return node_attributes_;
}

Attributes&
Graph::edgeAttributes()
{
    return edge_attributes_;
}

Subgraph&
Graph::addSubgraph( std::string id )
{
    subgraphs_.push_back( std::make_unique<Subgraph>( std::move( id ) ) );
    return *subgraphs_.back();
}

Node&
Graph::addNode( std::string id )
{
    nodes_.push_back( std::make_unique<Node>( std::move( id ) ) );
    return *nodes_.back();
}

Edge&
Graph::addEdge( std::string from, std::string to )
{
    edges_.push_back( std::make_unique<Edge>( std::move( from ), std::move( to ) ) );
    return *edges_.back();
}

Graph::
operator std::string() const
{
    std::ostringstream os;

    os << "digraph \"" << id_ << "\" {\n";

    if ( !graph_attributes_.empty() )
    {
        indent( os, 2 );
        os << "graph ";
        graph_attributes_.write( os );
        os << ";\n";
    }

    if ( !node_attributes_.empty() )
    {
        indent( os, 2 );
        os << "node ";
        node_attributes_.write( os );
        os << ";\n";
    }

    if ( !edge_attributes_.empty() )
    {
        indent( os, 2 );
        os << "edge ";
        edge_attributes_.write( os );
        os << ";\n";
    }

    for ( const auto& subgraph : subgraphs_ )
    {
        subgraph->write( os );
    }

    for ( const auto& node : nodes_ )
    {
        node->write( os );
    }

    for ( const auto& edge : edges_ )
    {
        edge->write( os );
    }

    os << "}\n";

    return os.str();
}

} // namespace dot_graph
