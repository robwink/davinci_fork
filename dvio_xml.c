#include "ff_struct.h"
#include "dvio.h"
#include "func.h"
#include <errno.h>
#include <stdio.h>
#include <string.h>

#ifdef HAVE_LIBXML2
#include "dvio_pds4.h"
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <libxml/xmlmemory.h>
#include <libxml/xmlreader.h>

Var* dv_LoadXML(char* filename, int use_names);

/**
 * This function is based on a similar function elsewhere in davinci.
 * The input string is converted to a davinci double, float, or int
 * struct if possible.
 *
 */
static Var* str_to_num(char* s)
{
	Var* v      = NULL;
	char* p     = s;
	long val    = 0;
	double dval = 0.0;

	errno = 0;

	// try checking for an int first
	val = strtol(s, &p, 10); // only handles base 10
	if (errno != 0 || s == p || *p != 0) {
		// check for a double
		errno = 0;
		p     = s;
		dval  = strtod(s, &p);
		if (errno != 0 || s == p || *p != 0) {
			// ok its neither an int nor a float
			v = NULL;
		} else {
			// s is a double
			if (dval > FLT_MAX) {
				v = newDouble(dval);
			} else {
				v = newFloat((float)dval);
			}
		}
	} else {
		// s is an int
		v = newInt((int)val);
	}
	return v;
}

/**
 * This function is based on a similar function elsewhere in davinci.
 * The input string is converted to a davinci string struct if possible.
 *
 */
static Var* str_to_Var(char* s)
{
	Var* v = NULL;

	if ((v = str_to_num(s)) == NULL) {
		return newString(s);
	} else {
		return v;
	}
}

static int get_attributes(xmlNodePtr nptr, Var* v)
{
	xmlAttrPtr attr    = NULL;
	char* attr_name    = NULL;
	char* attr_content = NULL;
	int rc             = 0;

	for (attr = nptr->properties; attr != NULL; attr = attr->next) {
		attr_name = strdup((char*)attr->name);
		if (attr_name == NULL) {
			rc = -1;
			break;
		}
		// don't assume there is content!!!
		if (attr->children->content == NULL) {
			rc = 0;
			break;
		}
		attr_content = strdup((char*)attr->children->content);
		if (attr_content == NULL) {
			free(attr_name);
			rc = 0;
			break;
		} else {
			add_struct(v, attr_name, str_to_Var(attr_content));
			rc = 1;
		}
	}
	return rc;
}

/**
 * This function searches an XML element node for a name attribute.
 * If found it is returned with the intent it will be used to create
 * the davinci struct representing the XML node.
 */
static char* get_name(xmlNodePtr nptr)
{
	char* name     = NULL;
	xmlNodePtr cur = NULL;

	cur = nptr->xmlChildrenNode;
	while (cur != NULL) {
		if (!strcmp((char*)cur->name, NAME_ATTRIBUTE)) {
			// we found the name of the element
			if (cur->children->content != NULL && cur->children->type == XML_TEXT_NODE) {
				name = strdup((char*)cur->children->content);
				if (name == NULL) {
					/* do something sensible here */
					parse_error("Memory allocation error processing an XML node %s\n", (char*)nptr->name);
					return NULL;
				}
				break;
			}
		}
		cur = cur->next;
	}
	return name;
}

/**
 * This function searches the input davinci struct for the existence of a
 * node with name <keyname>. If one is found an integer is appended to
 * keyname and the search repeated. This will continue until no match is
 * found or max_ser_no is reached. If successful, the unique keyname is
 * returned otherwise a NULL is returned.
 */
static char* gen_next_unused_name_instance(char* keyname, Var* s)
{
	char* ser_key_name;
	char* org_key_name;
	int i;
	int max_ser_no = 1000;
	Var* v;

	org_key_name = strdup(keyname);

	if (find_struct(s, org_key_name, &v) < 0) {
		// the name is currently unused so lets use it as is
		return org_key_name;
	}

	free(org_key_name);

	/* alloc a ridiculously large key name buffer */
	ser_key_name = (char*)calloc(strlen(keyname) + 64, sizeof(char));

	for (i = 1; i < max_ser_no; i++) {
		/* generate a key with the next free serial number */
		sprintf(ser_key_name, "%s_%d", keyname, i);

		if (find_struct(s, ser_key_name, &v) < 0) {
			/* if this serial number is unused, return this key */
			return ser_key_name;
		}
	}

	free(ser_key_name);

	return NULL; /* no such instance found */
}

/**
 * This function loads a PDS4 XML element node into a davinci struct.
 */
static int load_node(xmlNodePtr nptr, Var* v, int use_names)
{

	Var* elem               = NULL;
	Var* parent             = NULL;
	Var* node_data          = NULL;
	char* key_name          = NULL;
	char* node_name         = NULL;
	char* node_content      = NULL;
	char* text_content      = NULL;
	int is_field_bin        = 0;
	int is_group_bin        = 0;
	int is_table_bin        = 0;
	unsigned long child_cnt = 0;

	if (nptr->type == XML_ELEMENT_NODE && nptr->name != NULL) {
		node_name = strdup((char*)nptr->name);
		if (node_name == NULL) {
			parse_error("Memory allocation error processing an XML node %s\n", (char*)nptr->name);
			return 0;
		}
		if (!strcmp(node_name, FIELD_BINARY)) {
			if (use_names) {
				node_name = get_name(nptr);
				if (node_name == NULL) {
					return 0;
				}
			} else {
				node_name = gen_next_unused_name_instance(node_name, v);
			}
			is_field_bin = 1;
		} else if (!strcmp(node_name, GROUP_FIELD_BINARY)) {
			node_name = gen_next_unused_name_instance(node_name, v);
			if (node_name == NULL) {
				return 0;
			}

			is_field_bin = 0;
			is_group_bin = 1;
		} else if (!strcmp(node_name, TABLE_BINARY)) {
			if (use_names) {
				node_name = get_name(nptr);
				if (node_name == NULL) {
					return 0;
				}
				is_table_bin = 1;
			} else {
				node_name = gen_next_unused_name_instance(node_name, v);
			}
		} else if (!strcmp(node_name, FILE_AREA_OBSERVATIONAL)) {
			node_name = gen_next_unused_name_instance(node_name, v);

			if (node_name == NULL) {
				return 0;
			}
		} else if (use_names && !strcmp(node_name, NAME_ATTRIBUTE) &&
		           (!strcmp((char*)nptr->parent->name, TABLE_BINARY) ||
		            !strcmp((char*)nptr->parent->name, FIELD_BINARY))) {
			goto NEXT_NODE;
		}

		// the attributes of the node
		child_cnt = xmlChildElementCount(nptr);

		if (nptr->properties != NULL) {
			elem = new_struct(0);
			add_struct(v, node_name, elem);
			get_attributes(nptr, elem);
		}

		if (child_cnt < 1) {
			// don't assume there is content!!!
			if (nptr->children == NULL || nptr->children->content == NULL) {
				// using a zero length string here to deal with XML
				// elements with empty content...a nasty little hack
				node_content = strdup("");
			} else {
				node_content = strdup((char*)nptr->children->content);
			}
			if (node_content != NULL) {
				if (!strcmp(node_name, MD5_CHECKSUM)) {
					node_data = newString(node_content);
				} else {
					node_data = str_to_Var(node_content);
				}

				if (elem != NULL) {
					if (node_data != NULL) {
						add_struct(elem, "value", node_data);
					} else {
						parse_error("Error adding a 'value' sub-struct to %s\n", node_name);
						if (node_name != NULL) {
							free(node_name);
						}
						return 0;
					}
				} else {
					if (node_data != NULL) {
						add_struct(v, node_name, node_data);
					} else {
						parse_error("Error adding a 'value' sub-struct to %s\n", node_name);
						if (node_name != NULL) {
							free(node_name);
						}
						return 0;
					}
				}
			}
		} else {
			if (child_cnt > 0) {
				if (elem == NULL) {
					elem = new_struct(0);
					add_struct(v, node_name, elem);
				}

				// if we are using name child elements to name davinci nodes
				// instead of the actual XML element name we need to create
				// a sub-struct that describes what type of PDS4 element the
				// struct is.
				if (use_names && is_table_bin) {
					add_struct(elem, "element_type", newString(strdup(TABLE_BINARY)));
				} else if (use_names && is_field_bin) {
					add_struct(elem, "element_type", newString(strdup(FIELD_BINARY)));
				}
				load_node(nptr->children, elem, use_names);
			}
		}
	}

NEXT_NODE:
	if (nptr->next != NULL) {
		load_node(nptr->next, v, use_names);
	}

	return 1;
}

/**
 * This function parses a PDS4 label file into a davinci struct.
 */
static Var* load_xml_struct(char* filename, xmlDocPtr doc, Var* v, int use_names)
{
	xmlAttrPtr attr = NULL;

	if (doc != NULL) {
		v                   = new_struct(0);
		xmlNodePtr node_ptr = xmlDocGetRootElement(doc);

		xmlInitMemory();
		load_node(node_ptr, v, use_names);
	}

	return v;
}

/**
 * This function uses libmxl2 to read the PDS4 label into a libxml2 linked list
 * and then convert the linked list to a davinci struct.
 */
static Var* load_xml_func(const char* filename, xmlDocPtr doc /* the resulting document tree */,
                          Var* v, int use_names)
{
	xmlParserCtxtPtr ctxt; /* the parser context */
	/* create a parser context */
	ctxt = xmlNewParserCtxt();
	if (ctxt == NULL) {
		parse_error("Failed to allocate parser context\n");
		return v;
	}
	/* parse the file, activating the DTD validation option */
	doc = xmlCtxtReadFile(ctxt, filename, NULL, XML_PARSE_DTDATTR /* XML_PARSE_DTDVALID */);
	/* check if parsing succeeded */
	if (doc == NULL) {
		parse_error("Failed to parse %s\n", filename);
		return NULL;
	} else {
		/* check if validation succeeded */
		if (ctxt->valid == 0) {
			parse_error("Failed to validate %s\n", filename);
		}
	}

	return load_xml_struct(filename, doc, v, use_names);
}

/**
 * This function will attempt to read and parse an XML file.
 * If successful, the resulting data will be stored in a
 * davinci struct tree (map) mirroring the hierarchy of the
 * original XML.
 *
 * This was primarily created to read PDS4 label files. It will
 * also work to a very limited extent with any generic XML file.
 *
 * @param filename - name of the XML file to be read
 * @param use_names - if non-zero the code will attempt to
 * use a child name element if available to avoid name collisions
 * in the davinci struct. Otherwise, XML elements that have the
 * same name will retain that name with the addition of an
 * integer suffix.
 *
 * libxml2 must be installed on the system and linked in when
 * davinci is built.
 *
 * Should be thread safe...maybe...
 */
Var* dv_LoadXML(char* filename, int use_names)
{
	Var* v = NULL;
	xmlDocPtr doc;

	xmlKeepBlanksDefault(0);
	v = load_xml_func(filename, doc, v, use_names);

	xmlCleanupParser();

	return (v);
}
#endif
