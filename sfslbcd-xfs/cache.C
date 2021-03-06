#include <sys/param.h>
#include <sys/mount.h>
#include "cache.h"
#include "xfs-nfs.h"
#include "dmalloc.h"

bool 
xfs_fheq (xfs_handle x1, xfs_handle x2) 
{
  if (xfs_handle_eq(&x1, &x2)) 
    return true;
  else return false;
}

bool
nfs_fheq (nfs_fh3 n1, nfs_fh3 n2)
{
  return n1.data == n2.data;
}

str
dirname (uint32 index)
{
  return strbuf ("cache/%02X", index >> 8);
}

str 
filename (uint32 index)
{
  return strbuf ("cache/%02X/%02X", index >> 8, index & 0xff);
}

cache_entry *
update_cache (nfs_fh3 fh, ex_fattr3 attr, char *name) 
{
  //Also creates a new entry out of 'fh' if it's not already in cache
  cache_entry *e = nfsindex[fh];
  if (!e) {
    e = New cache_entry(fh, attr, name);
  } else {
    if (name)
      e->name = strbuf ("%s", name);
    e->nfs_attr = attr;
  }
  return e;
}

str 
setcache_name (uint32 index)
{

  str fname = filename (index);
  int fd = open (fname, O_CREAT | O_RDWR | O_TRUNC, 0666);
  if (fd < 0) {
    if (errno == ENOENT) {
      str dname = dirname (index);
#if DEBUG > 0
      warn << "Creating dir: " << dname << "\n";
#endif
      if (mkdir (dname, 0777) < 0) {
#if DEBUG > 0
	warn << strerror (errno) << "(" << errno << ") mkdir " 
	     << dname << "\n";
#endif
	return NULL;
      }
      fd = open (fname, O_CREAT | O_RDWR | O_TRUNC, 0666);
      if (fd < 0) {
#if DEBUG > 0
	warn << strerror (errno) << "(" << errno << ") on file =" 
	     << fname << "\n";
#endif
	return NULL;
      }
      close (fd);
    }
    else {
#if DEBUG > 0
      warn << strerror (errno) << "(" << errno << ") on file =" 
	   << fname << "\n";
#endif
      return NULL;
    }
  }
  close (fd);
  return fname;
}

nfstime3 
max (nfstime3 mtime, nfstime3 ctime)
{
  if (mtime.seconds > ctime.seconds)
    return mtime;
  else 
    if (mtime.seconds < ctime.seconds) 
      return ctime;
    else 
      if (mtime.nseconds > ctime.nseconds)
	return mtime;
      else return ctime;
}

bool 
greater (nfstime3 a, nfstime3 b)
{
  if (a.seconds > b.seconds)
    return true;
  else if (a.seconds == b.seconds &&
	   a.nseconds > b.nseconds)
    return true;
  else
    return false;
}

int 
assign_cachefile (int fd, int seqnum, cache_entry *e, 
		  char *name, xfs_cache_handle *ch, int flags) 
{
  assert (e->cache_name.len() < 256);
  memmove (name, e->cache_name, e->cache_name.len()+1);

  int cfd = open (name, flags, 0666);
  if (cfd < 0) {
#if DEBUG > 0
    warn << seqnum << ":" << "open cachefile failed\n";
#endif
    xfs_reply_err(fd, seqnum, EIO);
    return -1;
  }

  fhandle_t cfh;
  if (getfh (name, &cfh)) {
#if DEBUG > 0
    warn << seqnum << ":" << "getfh failed\n";
#endif
    xfs_reply_err(fd, seqnum, EIO);
    return -1;
  }

  memmove (ch, &cfh, sizeof (cfh));
  return cfd;
}

void 
cache_entry::attr_enter (const nfs_fh3 &fh, const ex_fattr3 *a, const wcc_attr *w)
{
#if 0
  cache_entry *e = nfsindex[fh];
  if (!a) {
    if (e)
      e->attr.expire = 0;
  } else 
    if (!e)
      e = New cache_entry (fh, a);
    else {
      e->set (a, w);
      e->touch ();
    }
#endif
}
  
void 
cache_entry::flush_access (const nfs_fh3 &fh, sfs_aid aid)
{

}

void 
cache_entry::access_enter (const nfs_fh3 &fh, sfs_aid aid, uint32 mask, uint32 perm)
{

}
