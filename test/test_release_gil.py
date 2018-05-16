from gild import sleep_for_one_second

import threading
import timeit
import unittest


class TestReleaseGIL(unittest.TestCase):

    def test_sleep(self):
        """
        Ensure that keeping the GIL blocks other threads.
        """
        threads = []
        for i in range(50):
            threads.append(threading.Thread(target=sleep_for_one_second))

        start_time = timeit.default_timer()
        for thread in threads:
            thread.start()
        for thread in threads:
            thread.join() # Wait for completion!
        elapsed = timeit.default_timer() - start_time

        self.assertGreaterEqual(elapsed, 1.0)
        self.assertLess(elapsed, 2.0)


if __name__ == '__main__':
    unittest.main()
