# Goldfinch

Goldfinch is a small RDF triplestore written in C. RDF data is stored as subject,
predicate, and object triples. Strings are assigned numeric IDs so the
triplestore can compare and store values efficiently.

## Architecture

```text
main.c
	|
	+-- storage/   RDF data loading and in-memory storage
	|
	+-- engine/    Triple-pattern matching and query results
	|
	+-- csparql/   Planned SPARQL lexer and parser
```

### Storage

The `storage` module owns the RDF data model and its in-memory structures:

- `triplestore.c`: creates the triplestore, inserts triples, counts triples,
  and releases triplestore memory.
- `dictionary.c`: maps RDF strings to numeric IDs and maps IDs back to strings.
- `hashtable.c`: stores and finds dictionary entries.
- `fetcher.c`: loads triples from Turtle files using Raptor2.

Headers are in `storage/include` and implementations are in `storage/src`.

### Engine

The `engine` module searches the stored triples. It accepts a triple pattern
where each of subject, predicate, and object can be fixed or variable. It
returns matching triples in a dynamically allocated `Results` structure.

Headers are in `engine/include` and the implementation is in `engine/src`.

### CSparql

The `csparql` module provides the query language layer. It currently includes
tokenization, parsing, and execution for a basic `SELECT` query.

Current stages:

1. The lexer converts SPARQL text into tokens.
2. The parser converts tokens into a parsed query structure.
3. The executor converts the parsed triple pattern into an engine call.
4. The executor prints values for the selected variables.

Next planned stages:

1. Expand the grammar beyond `SELECT`, `WHERE`, variables, IRIs,
   braces, and triple-pattern dots.
2. Add prefix declarations.
3. Add `ASK`, `INSERT DATA`, and `DELETE DATA` operations.

Example target query:

```sparql
SELECT ?person ?friend
WHERE {
		?person <http://xmlns.com/foaf/0.1/knows> ?friend .
}
```

## Building

The project uses GCC, Raptor2, and C17. From the repository root, build with:

```text
gcc -g -I engine/include -I storage/include \
		-I C:/msys64/ucrt64/include \
		-I C:/msys64/ucrt64/include/raptor2 \
		main.c storage/src/dictionary.c storage/src/hashtable.c \
		storage/src/fetcher.c storage/src/triplestore.c engine/src/engine.c \
		-L C:/msys64/ucrt64/lib -lraptor2 -o triplestore.exe
```

The same source files and include paths are configured in `.vscode/tasks.json`.

## Methods Used

### Loading and storage

| Method              | Purpose                                                                       |
| ------------------- | ----------------------------------------------------------------------------- |
| `fetch_TripleChars` | Loads Turtle data into string triples.                                        |
| `create`            | Allocates the triplestore, dictionary, hash table, and triple array.          |
| `insert`            | Converts subject, predicate, and object strings to IDs and stores a `Triple`. |
| `dictionary_get_id` | Finds or creates an ID for an RDF string.                                     |
| `hashtable_find`    | Finds an existing string-to-ID entry.                                         |
| `count`             | Returns the number of stored triples.                                         |

### Tokenizing and parsing

| Method                     | Purpose                                             |
| -------------------------- | --------------------------------------------------- |
| `getTokens`                | Converts the query string into a `Tokens` array.    |
| `parseQuery`               | Builds a `ParsedQuery` from the tokens.             |
| `getType`                  | Identifies the query operation, currently `SELECT`. |
| `getTriplePattern`         | Extracts subject, predicate, and object terms.      |
| `getVariablesAndPositions` | Records selected variables and their positions.     |
| `addVariable`              | Adds a variable to the dynamic `Variables` array.   |
| `variablesArrayGrow`       | Doubles the variable array capacity when needed.    |

### Execution and cleanup

| Method                  | Purpose                                                |
| ----------------------- | ------------------------------------------------------ |
| `execute`               | Connects tokenization, parsing, and engine execution.  |
| `getTriples`            | Finds triples matching the parsed pattern.             |
| `addToTriples`          | Adds a matching triple to `Results`.                   |
| `dictionary_get_string` | Converts a numeric ID back to an RDF string.           |
| `free_results`          | Releases the result array and result structure.        |
| `triplestore_free`      | Releases storage, dictionary, hash table, and triples. |
| `fetch_free`            | Releases the loaded `TripleChars` array and strings.   |
