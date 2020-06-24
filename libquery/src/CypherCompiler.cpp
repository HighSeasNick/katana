#include "querying/CypherCompiler.h"

// helpers functions anonymous namespace
namespace {
// TODO memory leaks; no free of the char *
char* str_to_cstr(std::string str) {
  char* cstr = new char[str.length() + 1];
  strcpy(cstr, str.c_str());
  return cstr;
}

// TODO memory leaks; no free of the char *
char* str_to_cstr(unsigned int id) {
  std::string str = std::to_string(id);
  char* cstr      = new char[str.length() + 1];
  strcpy(cstr, str.c_str());
  return cstr;
}
} // end anonymous namespace

unsigned CypherCompiler::getNodeID(std::string str) {
  if (this->nodeIDs.find(str) == this->nodeIDs.end()) {
    this->nodeIDs[str] = this->numNodeIDs++;
  }
  return this->nodeIDs[str];
}

unsigned CypherCompiler::getAnonNodeID(CypherASTNode node) {
  if (this->anonNodeIDs.find(node) == this->anonNodeIDs.end()) {
    this->anonNodeIDs[node] = this->numNodeIDs++;
  }
  return this->anonNodeIDs[node];
}

unsigned CypherCompiler::getEdgeID(std::string str) {
  if (this->edgeIDs.find(str) == this->edgeIDs.end()) {
    this->edgeIDs[str] = this->numEdgeIDs++;
  }
  return this->edgeIDs[str];
}

unsigned CypherCompiler::getAnonEdgeID(CypherASTNode node) {
  if (this->anonEdgeIDs.find(node) == this->anonEdgeIDs.end()) {
    this->anonEdgeIDs[node] = this->numEdgeIDs++;
  }
  return this->anonEdgeIDs[node];
}

void CypherCompiler::compile_node_pattern_path(CypherASTNode element,
                                               MatchedNode& mn) {
  std::string strName, strID;

  auto nameNode = cypher_ast_node_pattern_get_identifier(element);
  std::string name;
  if (nameNode != NULL) {
    name = cypher_ast_identifier_get_name(nameNode);
  }

  // loop through all labels on ast node and compile the label string for
  // the node
  auto nlabels = cypher_ast_node_pattern_nlabels(element);
  if ((nlabels > 0) || (this->labels.find(name) != this->labels.end())) {
    // TODO there seems to be an assumption here that only one or
    // the other holds but not both: otherwise strName can grow
    // pretty large and may have duplicates depending on what is saved in the
    // labels map
    for (unsigned int i = 0; i < nlabels; ++i) {
      if (i > 0) {
        strName += ";";
      }
      auto label = cypher_ast_node_pattern_get_label(element, i);
      strName += cypher_ast_label_get_name(label);
    }

    if (this->labels.find(name) != this->labels.end()) {
      if (nlabels > 0) {
        strName += ";";
      }
      strName += this->labels[name];
    }
  } else {
    strName += "any";
  }

  // save an identifier of the node
  if (nameNode != NULL) {
    strID += std::to_string(this->getNodeID(name));
    // find if there's a limit on this name and save it
    if (this->contains.find(name) != this->contains.end()) {
      this->filters.push_back(str_to_cstr(this->contains[name]));
    } else {
      this->filters.push_back(str_to_cstr(""));
    }
  } else {
    strID += std::to_string(this->getAnonNodeID(element));
    this->filters.push_back(str_to_cstr(""));
  }

  // identifier of node
  mn.id = str_to_cstr(strID);
  // strName has the labels of the node as a semicolon separated string
  mn.name = str_to_cstr(strName);
}

void CypherCompiler::compile_rel_pattern_path(CypherASTNode element) {
  uint64_t timestamp;
  std::string label;

  auto nameNode = cypher_ast_rel_pattern_get_identifier(element);
  std::string name;
  if (nameNode != NULL) {
    name = cypher_ast_identifier_get_name(nameNode);
  }
  auto nlabels        = cypher_ast_rel_pattern_nreltypes(element);
  unsigned int repeat = 1;

  auto varlength = cypher_ast_rel_pattern_get_varlength(element);
  if (varlength != NULL) {
    auto start = cypher_ast_range_get_start(varlength);
    auto end   = cypher_ast_range_get_end(varlength);
    if ((start == NULL) || (end == NULL)) {
      if (this->shortestPath) {
        label += "*";
        this->shortestPath = false;
      } else {
        label += "**"; // all paths; TODO: modify runtime to handle it
      }
      if (this->pathConstraints.find(this->namedPath) !=
          this->pathConstraints.end()) {
        label += "=";
        label += this->pathConstraints[this->namedPath];
        this->namedPath.clear();
      } else if (this->pathConstraints.find(name) !=
                 this->pathConstraints.end()) {
        label += "=";
        label += this->pathConstraints[name];
      } else if (nlabels > 0) {
        label += "=";
      }
    } else if (start == end) {
      repeat = atoi(cypher_ast_integer_get_valuestr(start));
    }
  }
  if (nlabels > 0) {
    for (unsigned int i = 0; i < nlabels; ++i) {
      if (i > 0) {
        label += ";";
      }
      auto reltype = cypher_ast_rel_pattern_get_reltype(element, i);
      label += cypher_ast_reltype_get_name(reltype);
    }
  }
  if (((varlength == NULL) || (repeat > 1)) && (nlabels == 0)) {
    label += "ANY";
  }

  if (nameNode != NULL) {
    if (this->timestamps.find(name) != this->timestamps.end()) {
      timestamp = this->timestamps[name];
    } else {
      timestamp = std::numeric_limits<uint32_t>::max();
    }
  } else {
    timestamp = std::numeric_limits<uint32_t>::max();
  }

  this->queryEdges.back().label     = str_to_cstr(label);
  this->queryEdges.back().timestamp = timestamp;

  for (unsigned int r = 1; r < repeat; ++r) {
    this->queryEdges.back().acted_on.id   = str_to_cstr(getAnonNodeID(element));
    this->queryEdges.back().acted_on.name = str_to_cstr("any");
    this->filters.push_back(str_to_cstr(""));

    this->queryEdges.emplace_back();
    this->queryEdges.back().caused_by.id   = str_to_cstr(getAnonNodeID(element));
    this->queryEdges.back().caused_by.name = str_to_cstr("any");
    this->filters.push_back(str_to_cstr(""));

    this->queryEdges.back().label     = str_to_cstr(label);
    this->queryEdges.back().timestamp = timestamp;
  }
}

int CypherCompiler::compile_pattern_path(CypherASTNode ast) {
  unsigned int nelements = cypher_ast_pattern_path_nelements(ast);

  if (nelements > 2) {
    // if greater than 2, it means it must have an edge and another
    // node; such constructions must ultimately have an odd number of
    // elements else there will be a disconnected node
    assert((nelements % 2) == 1); // odd number of elements

    // + 2 because we're interested in edges, which are every other element
    for (unsigned int i = 1; i < nelements; i += 2) {
      // get edge and its direction
      auto rel = cypher_ast_pattern_path_get_element(ast, i);
      assert(cypher_astnode_type(rel) == CYPHER_AST_REL_PATTERN);
      auto direction = cypher_ast_rel_pattern_get_direction(rel);

      // get the 2 nodes connected by the edge
      auto first = cypher_ast_pattern_path_get_element(ast, i - 1);
      assert(cypher_astnode_type(first) == CYPHER_AST_NODE_PATTERN);
      auto second = cypher_ast_pattern_path_get_element(ast, i + 1);
      assert(cypher_astnode_type(second) == CYPHER_AST_NODE_PATTERN);

      this->queryEdges.emplace_back(); // create memory for the edge

      // fill in nodes of edge based on direction
      if (direction == CYPHER_REL_OUTBOUND) { // source
        compile_node_pattern_path(first, this->queryEdges.back().caused_by);
        compile_rel_pattern_path(rel);
        compile_node_pattern_path(second, this->queryEdges.back().acted_on);
      } else {
        compile_node_pattern_path(first, this->queryEdges.back().acted_on);
        compile_rel_pattern_path(rel);
        compile_node_pattern_path(second, this->queryEdges.back().caused_by);
      }

      // if edge was bidirectional, create the node in the other direction
      if (direction == CYPHER_REL_BIDIRECTIONAL) {
        this->queryEdges.emplace_back();
        compile_node_pattern_path(first, this->queryEdges.back().caused_by);
        compile_rel_pattern_path(rel);
        compile_node_pattern_path(second, this->queryEdges.back().acted_on);
      }
    }
    return 0;
  } else {
    // single node pattern path
    this->queryNodes.emplace_back();
    auto node = cypher_ast_pattern_path_get_element(ast, 0);
    compile_node_pattern_path(node, this->queryNodes.back());
    return 0;
  }
}

void CypherCompiler::compile_binary_operator(CypherASTNode ast,
                                             bool negate = false) {
  auto op   = cypher_ast_binary_operator_get_operator(ast);
  auto arg1 = cypher_ast_binary_operator_get_argument1(ast);
  auto arg2 = cypher_ast_binary_operator_get_argument2(ast);
  if (op == CYPHER_OP_AND) {
    this->bin_op.push(true);
    compile_expression(arg1);
    compile_expression(arg2);
    this->bin_op.pop();
  } else if (op == CYPHER_OP_OR) {
    this->bin_op.push(false);
    compile_expression(arg1);
    compile_expression(arg2);
    this->bin_op.pop();
  } else if (op == CYPHER_OP_CONTAINS) {
    auto arg1_type = cypher_astnode_type(arg1);
    auto arg2_type = cypher_astnode_type(arg2);
    if ((arg1_type == CYPHER_AST_PROPERTY_OPERATOR) &&
        (arg2_type == CYPHER_AST_STRING)) {
      auto prop_id   = cypher_ast_property_operator_get_expression(arg1);
      auto prop_name = cypher_ast_property_operator_get_prop_name(arg1);
      if ((prop_id != NULL) && (prop_name != NULL)) {
        auto name = cypher_ast_prop_name_get_value(prop_name);
        if (!strcmp(name, "name")) {
          auto id    = cypher_ast_identifier_get_name(prop_id);
          auto value = cypher_ast_string_get_value(arg2);
          if (this->contains.find(id) == this->contains.end()) {
            if (negate) {
              this->contains[id] = std::string("((?!") + value + ").)*";
            } else {
              this->contains[id] = std::string("(.*") + value + ".*)";
            }
          } else {
            if (negate) {
              this->contains[id] =
                  std::string("((?!") + value + ").)*" + this->contains[id];
            } else if (this->bin_op.top()) {
              this->contains[id] =
                  std::string("(?=.*") + value + ".*)" + this->contains[id];
            } else {
              this->contains[id] =
                  std::string("(.*") + value + ".*)|" + this->contains[id];
            }
          }
        }
      }
    }
  } else if (op == CYPHER_OP_REGEX) {
    auto arg1_type = cypher_astnode_type(arg1);
    auto arg2_type = cypher_astnode_type(arg2);
    if ((arg1_type == CYPHER_AST_PROPERTY_OPERATOR) &&
        (arg2_type == CYPHER_AST_STRING)) {
      auto prop_id   = cypher_ast_property_operator_get_expression(arg1);
      auto prop_name = cypher_ast_property_operator_get_prop_name(arg1);
      if ((prop_id != NULL) && (prop_name != NULL)) {
        auto name = cypher_ast_prop_name_get_value(prop_name);
        if (!strcmp(name, "name")) {
          auto id    = cypher_ast_identifier_get_name(prop_id);
          auto value = cypher_ast_string_get_value(arg2);
          assert(this->contains.find(id) == this->contains.end());
          if (negate) {
            this->contains[id] = std::string("((?!") + value + ").)*";
          } else {
            this->contains[id] = value;
          }
        }
      }
    }
  }
}

void CypherCompiler::compile_comparison(CypherASTNode ast) {
  if (cypher_ast_comparison_get_length(ast) == 1) {
    auto arg1      = cypher_ast_comparison_get_argument(ast, 0);
    auto arg2      = cypher_ast_comparison_get_argument(ast, 1);
    auto arg1_type = cypher_astnode_type(arg1);
    auto arg2_type = cypher_astnode_type(arg2);
    if ((arg1_type == CYPHER_AST_PROPERTY_OPERATOR) &&
        (arg2_type == CYPHER_AST_PROPERTY_OPERATOR)) {
      auto prop_name1 = cypher_ast_property_operator_get_prop_name(arg1);
      auto prop_name2 = cypher_ast_property_operator_get_prop_name(arg2);
      if ((prop_name1 != NULL) && (prop_name2 != NULL)) {
        auto name1 = cypher_ast_prop_name_get_value(prop_name1);
        auto name2 = cypher_ast_prop_name_get_value(prop_name2);
        if (!strcmp(name1, "time") && !strcmp(name2, "time")) {
          auto prop_id1 = cypher_ast_property_operator_get_expression(arg1);
          auto prop_id2 = cypher_ast_property_operator_get_expression(arg2);
          auto id1      = cypher_ast_identifier_get_name(prop_id1);
          auto id2      = cypher_ast_identifier_get_name(prop_id2);

          auto op = cypher_ast_comparison_get_operator(ast, 0);
          // TODO: make it more general - topological sort among all timestamp
          // constraints
          if ((op == CYPHER_OP_LT) || (op == CYPHER_OP_LTE)) {
            if (this->timestamps.find(id1) == this->timestamps.end()) {
              if (this->timestamps.find(id2) == this->timestamps.end()) {
                this->timestamps[id1] = 5;
                this->timestamps[id2] = 10;
              } else {
                this->timestamps[id1] = this->timestamps[id2] - 1;
              }
            } else {
              if (this->timestamps.find(id2) == this->timestamps.end()) {
                this->timestamps[id2] = this->timestamps[id1] + 1;
              } else {
                assert(this->timestamps[id1] <= this->timestamps[id2]);
              }
            }
          } else if ((op == CYPHER_OP_GT) || (op == CYPHER_OP_GTE)) {
            if (this->timestamps.find(id1) == this->timestamps.end()) {
              if (this->timestamps.find(id2) == this->timestamps.end()) {
                this->timestamps[id1] = 10;
                this->timestamps[id2] = 5;
              } else {
                this->timestamps[id1] = this->timestamps[id2] + 1;
              }
            } else {
              if (this->timestamps.find(id2) == this->timestamps.end()) {
                this->timestamps[id2] = this->timestamps[id1] - 1;
              } else {
                assert(this->timestamps[id1] >= this->timestamps[id2]);
              }
            }
          }
        }
      }
    }
  }
}

void CypherCompiler::compile_labels_operator(CypherASTNode ast,
                                             std::string prefix = "") {
  auto labels_id = cypher_ast_labels_operator_get_expression(ast);
  auto id        = cypher_ast_identifier_get_name(labels_id);
  for (unsigned int i = 0; i < cypher_ast_labels_operator_nlabels(ast); ++i) {
    auto label = cypher_ast_labels_operator_get_label(ast, i);
    auto name  = cypher_ast_label_get_name(label);
    if (this->labels.find(id) == this->labels.end()) {
      this->labels[id] = prefix + name;
    } else {
      this->labels[id] += ";" + prefix + name; // TODO: fix assumption of AND
    }
  }
}

void CypherCompiler::compile_unary_operator(CypherASTNode ast) {
  auto op = cypher_ast_unary_operator_get_operator(ast);
  if (op == CYPHER_OP_NOT) {
    auto arg      = cypher_ast_unary_operator_get_argument(ast);
    auto arg_type = cypher_astnode_type(arg);
    if (arg_type == CYPHER_AST_LABELS_OPERATOR) {
      compile_labels_operator(arg, "~");
    } else if (arg_type == CYPHER_AST_BINARY_OPERATOR) {
      compile_binary_operator(arg, true);
    }
  }
}

void CypherCompiler::compile_list_comprehension(CypherASTNode ast,
                                                std::string prefix = "") {
  auto list_id = cypher_ast_list_comprehension_get_identifier(ast);
  auto id      = cypher_ast_identifier_get_name(list_id);
  auto new_id  = id;

  auto expression = cypher_ast_list_comprehension_get_expression(ast);
  auto exp_type   = cypher_astnode_type(expression);
  if (exp_type == CYPHER_AST_APPLY_OPERATOR) {
    // auto exp_fn = cypher_ast_apply_operator_get_func_name(expression);
    // assume function is inverse of the other
    auto arg      = cypher_ast_apply_operator_get_argument(expression, 0);
    auto arg_type = cypher_astnode_type(arg);
    if (arg_type == CYPHER_AST_IDENTIFIER) {
      new_id = cypher_ast_identifier_get_name(arg);
    }
  } else if (exp_type == CYPHER_AST_IDENTIFIER) {
    new_id = cypher_ast_identifier_get_name(expression);
  }

  auto predicate = cypher_ast_list_comprehension_get_predicate(ast);
  auto pred_type = cypher_astnode_type(predicate);
  if (pred_type == CYPHER_AST_BINARY_OPERATOR) {
    auto op = cypher_ast_binary_operator_get_operator(predicate);
    if (op == CYPHER_OP_EQUAL) {
      auto arg1      = cypher_ast_binary_operator_get_argument1(predicate);
      auto arg2      = cypher_ast_binary_operator_get_argument2(predicate);
      auto arg1_type = cypher_astnode_type(arg1);
      auto arg2_type = cypher_astnode_type(arg2);
      if (arg1_type == CYPHER_AST_APPLY_OPERATOR) {
        // auto arg1_fn = cypher_ast_apply_operator_get_func_name(arg1);
        // assume function is inverse of the other
        auto arg      = cypher_ast_apply_operator_get_argument(arg1, 0);
        auto arg_type = cypher_astnode_type(arg);
        if (arg_type == CYPHER_AST_IDENTIFIER) {
          if (!strcmp(id, cypher_ast_identifier_get_name(arg))) {
            if (arg2_type == CYPHER_AST_STRING) {
              this->pathConstraints[new_id] =
                  prefix + cypher_ast_string_get_value(arg2);
            }
          }
        }
      }
    }
  }
}

void CypherCompiler::compile_none(CypherASTNode ast) {
  compile_list_comprehension(ast, "~");
}

void CypherCompiler::compile_expression(CypherASTNode ast) {
  auto type = cypher_astnode_type(ast);
  if (type == CYPHER_AST_BINARY_OPERATOR) {
    compile_binary_operator(ast);
  } else if (type == CYPHER_AST_COMPARISON) {
    compile_comparison(ast);
  } else if (type == CYPHER_AST_UNARY_OPERATOR) {
    compile_unary_operator(ast);
  } else if (type == CYPHER_AST_LABELS_OPERATOR) {
    compile_labels_operator(ast);
  } else if (type == CYPHER_AST_NONE) {
    compile_none(ast);
  }
}

int CypherCompiler::compile_ast_projection(CypherASTNode) {
  // TODO
  return -1;
}

int CypherCompiler::compile_ast_return(CypherASTNode returnAST) {
  // get number of projections and handle each separately
  unsigned numProjections = cypher_ast_return_nprojections(returnAST);
  for (unsigned i = 0; i < numProjections; i++) {
    CypherASTNode projectionAST =
        cypher_ast_return_get_projection(returnAST, i);
    int result = this->compile_ast_projection(projectionAST);
    if (result < 0) {
      galois::gDebug("projection under return AST failed to parse");
      return -1;
    }
  }

  // TODO
  // handle DISTINCT
  // handle ORDER BY
  // handle LIMIT
  // handle SKIP
  return 0;
}

int CypherCompiler::compile_ast_node(CypherASTNode ast) {
  // get the type of the node
  auto type = cypher_astnode_type(ast);

  // TODO seems like

  // handle differently based on what the type is
  if (type == CYPHER_AST_MATCH) {
    galois::gDebug("Compiling AST node match");
    // match has the following up for grabs
    // - pattern
    // - predicate
    // - optional tag
    // get predicate if it exists (i.e. WHERE ...)
    auto predicate = cypher_ast_match_get_predicate(ast);
    if (predicate != NULL)
      compile_expression(predicate);

    auto pattern = cypher_ast_match_get_pattern(ast);
    if (pattern != NULL) {
      // match on the pattern
      return compile_ast_node(pattern);
    } else {
      // shouldn't get here; means that the match has nothing to match on
      return 0;
    }
  } else if (type == CYPHER_AST_PATTERN_PATH) {
    galois::gDebug("Compiling AST pattern path");
    return compile_pattern_path(ast);
  } else if (type == CYPHER_AST_SHORTEST_PATH) {
    galois::gDebug("Compiling AST node shortest path");
    this->shortestPath = true;
    assert(cypher_ast_shortest_path_is_single(ast));
    return compile_ast_node(cypher_ast_shortest_path_get_path(ast));
  } else if (type == CYPHER_AST_NAMED_PATH) {
    galois::gDebug("Compiling AST node named path");
    auto named_id   = cypher_ast_named_path_get_identifier(ast);
    this->namedPath = cypher_ast_identifier_get_name(named_id);

    auto path = cypher_ast_named_path_get_path(ast);
    return compile_ast_node(path);
  } else if (type == CYPHER_AST_RETURN) {
    galois::gDebug("Compiling AST node return");
    return compile_ast_return(ast);
  }

  // note: seems that assumption is that you only hit the below children
  // loop if there is no way to process the current ast node
  galois::gDebug("Compiling children nodes");
  for (unsigned int i = 0; i < cypher_astnode_nchildren(ast); ++i) {
    // TODO maybe track descent level to make debug statements more
    // easily parseable?
    CypherASTNode child = cypher_astnode_get_child(ast, i);
    if (compile_ast_node(child) < 0) {
      // error
      return -1;
    }
  }
  return 0;
}

int CypherCompiler::compile_ast(const cypher_parse_result_t* ast) {
  // loop over roots
  for (unsigned int i = 0; i < ast->nroots; ++i) {
    galois::gDebug("Handling root ", i);
    // handle each root recursively
    if (compile_ast_node(ast->roots[i]) < 0) {
      return -1;
    }
  }
  return 0;
}

void CypherCompiler::init() {
  this->numNodeIDs = 0;
  this->numEdgeIDs = 0;
  this->nodeIDs.clear();
  this->anonNodeIDs.clear();
  this->edgeIDs.clear();
  this->anonEdgeIDs.clear();
  this->contains.clear();
  this->timestamps.clear();
  this->labels.clear();
  this->pathConstraints.clear();
  this->shortestPath = false;
  this->namedPath.clear();
  this->queryEdges.clear();
  this->filters.clear();
}

int CypherCompiler::compile(const char* queryStr) {
  this->init();

  galois::gDebug("Query:\n", queryStr);

  cypher_parse_result_t* result =
      cypher_parse(queryStr, NULL, NULL, CYPHER_PARSE_ONLY_STATEMENTS);

  if (result == NULL) {
    galois::gError("Critical failure in parsing the cypher query\n");
    return EXIT_FAILURE;
  }

  auto nerrors = cypher_parse_result_nerrors(result);

  galois::gDebug("Parsed ", cypher_parse_result_nnodes(result), " AST nodes");
  galois::gDebug("Read ", cypher_parse_result_ndirectives(result),
                 " statements");
  galois::gDebug("Encountered ", nerrors, " errors");
#ifndef NDEBUG
  if (nerrors == 0) {
    cypher_parse_result_fprint_ast(result, stdout, 0, NULL, 0);
  }
#endif

  if (nerrors == 0) {
    // take ast, change it to a query graph
    this->compile_ast(result);
  }
  // free memory used by parser
  cypher_parse_result_free(result);
  galois::gInfo("Cypher query compilation complete");

  if (nerrors != 0) {
    galois::gError("Parsing the cypher query failed with ", nerrors, " errors");
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
