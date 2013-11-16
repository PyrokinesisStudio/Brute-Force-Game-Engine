rem Copy this file into boost root folder and run it. Probably you have to copy the lines into cmd and run it by hand.
rem IMPORTANT Don't forget to add boost geometry extension folder to /boost/geometry/extensions.
rem Revison: 77008
rem http://svn.boost.org/svn/boost/trunk/boost/geometry/extensions 

call bootstrap.bat
b2 --with-date_time --with-log --with-filesystem --with-graph --with-iostreams --with-program_options --with-random --with-regex --with-serialization --with-system --with-test --with-thread define=BOOST_TEST_DYN_LINK define=BOOST_TEST_MAIN threading=multi link=shared 