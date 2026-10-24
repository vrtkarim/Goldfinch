<p align="center">
  <img src="image.png" alt="Goldfinch" width="320">
</p>

# Goldfinch

Goldfinch is a small RDF triplestore written in C. It stores RDF data as
subject-predicate-object triples and is a project for learning how RDF storage
and SPARQL query processing work together.

> **Disclaimer:** This project is harder than I thought, but that is part of
> the learning process.

## How the data is stored

RDF data is kept in memory. Each triple contains a subject, predicate, and
object. Goldfinch gives every unique RDF string a numeric ID using a dictionary,
then stores those IDs in the triples. This avoids comparing long strings every
time the engine searches the data.

The storage layer is split into a few simple parts:

- `fetcher.c` loads triples from Turtle files using Raptor2.
- `dictionary.c` maps RDF strings to IDs and maps IDs back to strings.
- `hashtable.c` helps find dictionary entries.
- `triplestore.c` stores triples and manages the in-memory database.

## How SPARQL parsing works

The `csparql` directory contains the query-language code. A query moves through
three main steps:

1. The tokenizer reads the text and creates tokens such as `SELECT`, variables,
   IRIs, braces, dots, and prefixed names such as `ex:alice`.
2. The parser uses those tokens to build a structured query and find its triple
   pattern.
3. The executor sends the pattern to the engine, which searches the stored
   triples and prints the selected values.

For example:

```sparql
PREFIX ex: <http://example.com/>

SELECT ?person
WHERE {
    ?person ex:knows ex:alice .
}
```

The parser can also read a query with multiple required triple patterns and an
OPTIONAL block:

```sparql
PREFIX foaf: <http://xmlns.com/foaf/0.1/>

SELECT DISTINCT ?person
WHERE {
    ?person foaf:name ?name .
    ?personaa foaf:age ?name .

    OPTIONAL {
        ?person foaf:knows ?friend .
        ?person foaf:knows ?friend .
        ?person foaf:knows ?friend .
    }
}
GROUP BY ?person
HAVING (COUNT(?friend) > 0)
ORDER BY DESC(?friendCount)
LIMIT 10
OFFSET 0
```

The SELECT result for this query is:

```text
========== SELECT ==========
DISTINCT: true

--- Variables (1) ---
  var[0]: value=?person, isVariable=true

=== Triple Patterns (5) ===
Triple[0]: optional=false
  term[0]: value=?person, isVariable=true
  term[1]: value=http://xmlns.com/foaf/0.1/name, isVariable=false
  term[2]: value=?name, isVariable=true
Triple[1]: optional=false
  term[0]: value=?personaa, isVariable=true
  term[1]: value=http://xmlns.com/foaf/0.1/age, isVariable=false
  term[2]: value=?name, isVariable=true
Triple[2]: optional=true
  term[0]: value=?person, isVariable=true
  term[1]: value=http://xmlns.com/foaf/0.1/knows, isVariable=false
  term[2]: value=?friend, isVariable=true
Triple[3]: optional=true
  term[0]: value=?person, isVariable=true
  term[1]: value=http://xmlns.com/foaf/0.1/knows, isVariable=false
  term[2]: value=?friend, isVariable=true
Triple[4]: optional=true
  term[0]: value=?person, isVariable=true
  term[1]: value=http://xmlns.com/foaf/0.1/knows, isVariable=false
  term[2]: value=?friend, isVariable=true
```

## Current work

I am currently working on the SPARQL parser. My focus is making tokenizing and
parsing reliable for prefix declarations, prefixed names, variables, IRIs, and
triple patterns. Once the basic `SELECT` flow is solid, more query types such
as `ASK`, `INSERT`, and `DELETE` can be added.
And please make sure that the main file is used for testing features, doesnt have the implementation for the triplestore for now.

## Project layout

```text
main.c       Program entry point and query experiments
storage/     RDF loading and in-memory triple storage
engine/      Triple-pattern matching
csparql/     Tokenizing, parsing, and query execution
```
