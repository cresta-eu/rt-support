

#ifndef _TREE_INCLUDED_
#define _TREE_INCLUDED_


#include <map>
#include <list>
#include <string>

using std::string;
using std::list;
using std::map;

#include "rt_tasks.h"

/* ------------------------------------------------------------------------- */

#define PATH_SEP '/'

class string_path
{
  public:
    string_path(const char *path);
    ~string_path( void );

    void set( const char *path );

    string path_string( void );

    list<string> _path;
};


class task_tree
{
  public:

    task_tree( const char *key, rt_task *task );
    ~task_tree( void );


    task_tree *add_task( const char *path, rt_task *task );
    task_tree *find_node( const char *path );

    string   _key;
    rt_task *_task;

    list<task_tree *> _children;

};

/* ------------------------------------------------------------------------- */

#endif /* _TREE_INCLUDED_ */
