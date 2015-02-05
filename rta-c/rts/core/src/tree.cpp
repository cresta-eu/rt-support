
#include "tree.h"

#include <string>
#include <list>

#include "rt_tasks.h"

using namespace std;

/* ------------------------------------------------------------------------- */

#define EOS '\0'
#define PATH_SEP_STR "/"

string_path::string_path(const char *path)
{
    set(path);
}

string_path::~string_path( void )
{
}


void string_path::set( const char *path )
{
    if ( path == NULL )  return;

    int startC = 0;

    while ( path[startC] == PATH_SEP )
        startC += 1;

    for (int curC = startC; path[curC] != '\0'; curC++)
    {
        if ( path[curC] == PATH_SEP )
        {
            _path.push_back(string(path + startC, curC-startC));
            for (startC = curC; path[startC] == PATH_SEP; startC++)
                ;
            curC = startC;
        }
    }
    if ( path[startC] != EOS )
        _path.push_back(string(path + startC));

}

string string_path::path_string( void )
{
    string ret;
    list<string>::const_iterator curEl;

    for (curEl = _path.begin(); curEl != _path.end(); curEl++)
    {
        if ( ret.size() > 0 )
            ret.append(PATH_SEP_STR);
        ret.append(*curEl);
    }

    return ret;
}

/* ------------------------------------------------------------------------- */

task_tree::task_tree( const char *key, rt_task *task )
    : _key(key), _task(task)
{
}

task_tree::~task_tree( void )
{
    delete _task;

}

task_tree *task_tree::add_task( const char *path, rt_task *task )
{
    task_tree *curNode = this;

    string_path                   nP(path);
    list<string>::const_iterator  segName;

    for (segName = nP._path.begin(); segName != nP._path.end(); segName++)
    {
        task_tree *child = find_node((*segName).c_str());

        if ( child == NULL )
        {
            child = new task_tree((*segName).c_str(), NULL);
            curNode->_children.push_back(child);
        }
        curNode = child;
    }
    curNode->_task = task;

    return curNode;
}

task_tree *task_tree::find_node( const char *path )
{
    task_tree *curNode = this;

    string_path                   nP(path);
    list<string>::const_iterator  segName;

    for (segName = nP._path.begin(); segName != nP._path.end(); segName++)
    {
        list<task_tree *>::const_iterator child = curNode->_children.begin();

        for ( ; child != curNode->_children.end(); child++)
        {
            if ( (*child)->_key == *segName )
            {
                curNode = *child;
                break;
            }
        }
        if ( child == curNode->_children.end() )
        {
            curNode = NULL;
            break;
        }
    }

    return curNode;
}
