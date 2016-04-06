import unittest
import comma
import numpy
import numpy.testing

class test_time( unittest.TestCase ):
  def test_to_numpy( self ):
    f = comma.csv.time.to_numpy
    numpy.testing.assert_equal( f( '20150102T122345.012345' ), numpy.datetime64( '2015-01-02 12:23:45.012345', 'us' ) )
    numpy.testing.assert_equal( f( '20150102T122345' ), numpy.datetime64( '2015-01-02 12:23:45.000000', 'us' ) )
    numpy.testing.assert_equal( f( '' ), numpy.datetime64() )
    numpy.testing.assert_equal( f( 'not-a-date-time' ), numpy.datetime64() )
    self.assertEqual( f( '20150102T122345.012345' ).itemsize, 8 )
    invalid = '20150102'
    self.assertRaises( Exception, f, invalid )

  def test_from_numpy( self ):
    f = comma.csv.time.from_numpy
    numpy.testing.assert_equal( f( numpy.datetime64( '2015-01-02 12:23:45.012345', 'us' ) ), '20150102T122345.012345' )
    numpy.testing.assert_equal( f( numpy.datetime64( '2015-01-02 12:23:45.000000', 'us' ) ), '20150102T122345' )
    self.assertEqual( f( numpy.datetime64() ), 'not-a-date-time' )
    self.assertRaises( Exception, f, 'invalid' )

  def test_ascii_converters( self ):
    f = comma.csv.time.ascii_converters
    self.assertDictEqual( f( ( 'u1', 'u1', 'datetime64[us]', 'u1', 'M8[us]', 'u1' ) ), { 2: comma.csv.time.to_numpy, 4: comma.csv.time.to_numpy } )

  def test_zone( self ):
    def f( t ): return str( comma.csv.time.to_numpy( t ) )
    # note that the time zones used below do not have daylight saving time
    comma.csv.time.zone( 'Asia/Jakarta' )
    self.assertEqual( f( '20150101T000000' ), '2015-01-01T00:00:00.000000+0700' )
    comma.csv.time.zone( 'America/Caracas' )
    self.assertEqual( f( '20150101T000000' ), '2015-01-01T00:00:00.000000-0430' )
    comma.csv.time.zone( 'UTC' )
    self.assertEqual( f( '20150101T000000' ), '2015-01-01T00:00:00.000000+0000' )

if __name__ == '__main__':
    unittest.main()
