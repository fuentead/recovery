/*
 * FileAppender.cpp
 *
 * Copyright 2000, LifeLine Networks BV (www.lifeline.nl). All rights reserved.
 * Copyright 2000, Bastiaan Bakker. All rights reserved.
 *
 * (C) Copyright 2010-2014 Hewlett-Packard Development Company, L.P.
 *
 * See the COPYING file for the terms of usage and distribution.
 */

#include "PortabilityImpl.hh"
#ifdef LOG4CPP_HAVE_IO_H
#    include <io.h>
#endif
#ifdef LOG4CPP_HAVE_UNISTD_H
#    include <unistd.h>
#endif

#include <memory>
#include <stdio.h>
#include <time.h>
#include <log4cpp/FileAppender.hh>
#include <log4cpp/Category.hh>
#include <log4cpp/FactoryParams.hh>

namespace log4cpp {

    FileAppender::FileAppender(const std::string& name, 
                               const std::string& fileName,
                               bool append,
                               mode_t mode,
                               bool addPid) : 
            LayoutAppender(name),
            _fileName(appendPid(fileName, addPid)),
            _flags(O_CREAT | O_APPEND | O_WRONLY),
            _mode(mode) {
        if (!append)
            _flags |= O_TRUNC;
            
        _fd = ::open(_fileName.c_str(), _flags, _mode);
    }
    
    FileAppender::FileAppender(const std::string& name, int fd) :
        LayoutAppender(name),
        _fileName(""),
        _fd(fd),
        _flags(O_CREAT | O_APPEND | O_WRONLY),
        _mode(00644) {
    }
    
    FileAppender::~FileAppender() {
        close();
    }

    void FileAppender::close() {
        if (_fd!=-1) {
            off_t offset = ::lseek(_fd, 0, SEEK_END);
            	
            ::close(_fd);
            _fd=-1;
            
            if (offset==0) {
                // The log file is empty. Delete it.
                ::unlink(_fileName.c_str());
            }
        }
    }

    void FileAppender::setAppend(bool append) {
        if (append) {
            _flags &= ~O_TRUNC;
        } else {
            _flags |= O_TRUNC;
        }
    }

    bool FileAppender::getAppend() const {
        return (_flags & O_TRUNC) == 0;
    }

    void FileAppender::setMode(mode_t mode) {
        _mode = mode;
    }

    mode_t FileAppender::getMode() const {
        return _mode;
    }

    void FileAppender::_append(const LoggingEvent& event) {
        std::string message(_getLayout().format(event));
        if (!::write(_fd, message.data(), message.length())) {
            // XXX help! help!
        }
    }

    bool FileAppender::reopen() {
        if (_fileName != "") {
            int fd = ::open(_fileName.c_str(), _flags, _mode);
            if (fd < 0)
                return false;
            else {
	        if (_fd != -1)
                    ::close(_fd);
                _fd = fd;
                return true;
            }
        } else {
            return true;
        }      
    }

   std::string& FileAppender::appendPid(const std::string fileName, bool addPid)
   {
       // create a non-const filename object.
       static std::string nc_filename;
       nc_filename = fileName;
       
#ifdef LOG4CPP_HAVE_UNISTD_H
       if (addPid)
       {
           // Get the process ID.
           pid_t pid = getpid();
           char buff[10] = "";
           sprintf(buff, ".%d", pid);
       
           // Append the pid to the filename.
           nc_filename.append(buff);
       }
#endif
       
       return nc_filename;
   }
   	
   std::auto_ptr<Appender> create_file_appender(const FactoryParams& params)
   {
      std::string name, filename;
      bool append = true;
      mode_t mode = 664;

      params.get_for("file appender").required("name", name)("filename", filename)
                                     .optional("append", append)("mode", mode);

      return std::auto_ptr<Appender>(new FileAppender(name, filename, append, mode));
   }
}
