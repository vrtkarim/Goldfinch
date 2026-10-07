<p align="center">
  <img src="image.png" alt="Goldfinch" width="320">
</p>

# Goldfinch

Goldfinch is a small RDF triplestore written in C. It stores RDF data as
subject-predicate-object triples and is a project for learning how RDF storage
and SPARQL query processing work together.

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
