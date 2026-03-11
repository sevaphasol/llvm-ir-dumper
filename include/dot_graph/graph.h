#pragma once

#include <cstddef>
#include <memory>
#include <ostream>
#include <string>
#include <string_view>
#include <vector>

namespace dot_graph {

class Attributes {
  public:
    enum class ValueKind {
        QUOTED,
        RAW,
        HTML,
    };

    struct Entry
    {
        std::string key;
        std::string value;
        ValueKind   kind;

        void
        write( std::ostream& os, size_t width = 0 ) const;
    };

  private:
    std::vector<Entry> entries_;

  private:
    Entry&
    upsert( std::string_view key );

    Attributes&
    set( std::string_view key, std::string_view value, ValueKind kind );

  public:
    Attributes&
    setQuoted( std::string_view key, std::string_view value );

    Attributes&
    setRaw( std::string_view key, std::string_view value );

    Attributes&
    setHtml( std::string_view key, std::string_view value );

    template<typename TNum>
    Attributes&
    setNumber( std::string_view key, TNum num );

    Attributes&
    setBool( std::string_view key, bool value );

    Attributes&
    setQuotedLabel( std::string_view value );

    Attributes&
    setHtmlLabel( std::string_view value );

    Attributes&
    setColor( std::string_view value );

    Attributes&
    setFillColor( std::string_view value );

    Attributes&
    setFontColor( std::string_view value );

    Attributes&
    setFontName( std::string_view value );

    Attributes&
    setFontSize( std::size_t value );

    Attributes&
    setShape( std::string_view value );

    Attributes&
    setStyle( std::string_view value );

    Attributes&
    setPenWidth( std::size_t value );

    Attributes&
    setWeight( std::size_t value );

    Attributes&
    setConstraint( bool value );

    bool
    empty() const;

    const std::vector<Entry>&
    entries() const;

    void
    write( std::ostream& os, size_t width = 0 ) const;
};

template<typename Derived>
class AttributedElement {
  protected:
    Attributes attributes_;

  public:
    Attributes&
    attributes()
    {
        return attributes_;
    }

    const Attributes&
    attributes() const
    {
        return attributes_;
    }

    Derived&
    setQuoted( std::string_view key, std::string_view value )
    {
        attributes_.setQuoted( key, value );
        return static_cast<Derived&>( *this );
    }

    Derived&
    setRaw( std::string_view key, std::string_view value )
    {
        attributes_.setRaw( key, value );
        return static_cast<Derived&>( *this );
    }

    Derived&
    setHtml( std::string_view key, std::string_view value )
    {
        attributes_.setHtml( key, value );
        return static_cast<Derived&>( *this );
    }

    template<typename TNum>
    Derived&
    setNumber( std::string_view key, TNum value )
    {
        attributes_.setNumber( key, value );
        return static_cast<Derived&>( *this );
    }

    Derived&
    setBool( std::string_view key, bool value )
    {
        attributes_.setBool( key, value );
        return static_cast<Derived&>( *this );
    }

    Derived&
    setQuotedLabel( std::string_view value )
    {
        attributes_.setQuotedLabel( value );
        return static_cast<Derived&>( *this );
    }

    Derived&
    setHtmlLabel( std::string_view value )
    {
        attributes_.setHtmlLabel( value );
        return static_cast<Derived&>( *this );
    }

    Derived&
    setColor( std::string_view value )
    {
        attributes_.setColor( value );
        return static_cast<Derived&>( *this );
    }

    Derived&
    setFillColor( std::string_view value )
    {
        attributes_.setFillColor( value );
        return static_cast<Derived&>( *this );
    }

    Derived&
    setFontColor( std::string_view value )
    {
        attributes_.setFontColor( value );
        return static_cast<Derived&>( *this );
    }

    Derived&
    setFontName( std::string_view value )
    {
        attributes_.setFontName( value );
        return static_cast<Derived&>( *this );
    }

    Derived&
    setFontSize( std::size_t value )
    {
        attributes_.setFontSize( value );
        return static_cast<Derived&>( *this );
    }

    Derived&
    setShape( std::string_view value )
    {
        attributes_.setShape( value );
        return static_cast<Derived&>( *this );
    }

    Derived&
    setStyle( std::string_view value )
    {
        attributes_.setStyle( value );
        return static_cast<Derived&>( *this );
    }

    Derived&
    setPenWidth( std::size_t value )
    {
        attributes_.setPenWidth( value );
        return static_cast<Derived&>( *this );
    }

    Derived&
    setWeight( std::size_t value )
    {
        attributes_.setWeight( value );
        return static_cast<Derived&>( *this );
    }

    Derived&
    setConstraint( bool value )
    {
        attributes_.setConstraint( value );
        return static_cast<Derived&>( *this );
    }

    virtual void
    write( std::ostream& os, size_t width = 0 ) const = 0;
};

class Node : public AttributedElement<Node> {
  private:
    std::string id_;

  public:
    explicit Node( std::string id );

    const std::string&
    id() const;

    void
    write( std::ostream& os, size_t width = 0 ) const override;
};

class Edge : public AttributedElement<Edge> {
  private:
    std::string from_;
    std::string to_;

  public:
    Edge( std::string from, std::string to );

    const std::string&
    from() const;

    const std::string&
    to() const;

    void
    write( std::ostream& os, size_t width = 0 ) const override;
};

class Subgraph : public AttributedElement<Subgraph> {
  private:
    std::string                            id_;
    std::vector<std::unique_ptr<Subgraph>> subgraphs_;
    std::vector<std::unique_ptr<Node>>     nodes_;

  public:
    explicit Subgraph( std::string id );

    const std::string&
    id() const;

    Subgraph&
    addSubgraph( std::string id );

    Node&
    addNode( std::string id );

    const std::vector<std::unique_ptr<Subgraph>>&
    subgraphs() const;

    const std::vector<std::unique_ptr<Node>>&
    nodes() const;

    void
    write( std::ostream& os, size_t width = 0 ) const override;
};

class Graph {
  private:
    std::string                            id_;
    Attributes                             graph_attributes_;
    Attributes                             node_attributes_;
    Attributes                             edge_attributes_;
    std::vector<std::unique_ptr<Subgraph>> subgraphs_;
    std::vector<std::unique_ptr<Node>>     nodes_;
    std::vector<std::unique_ptr<Edge>>     edges_;

  public:
    explicit Graph( std::string id );

    Attributes&
    graphAttributes();

    Attributes&
    nodeAttributes();

    Attributes&
    edgeAttributes();

    Subgraph&
    addSubgraph( std::string id );

    Node&
    addNode( std::string id );

    Edge&
    addEdge( std::string from, std::string to );

    explicit
    operator std::string() const;
};

} // namespace dot_graph
