from gild import Control
from gild import minimal_worker_example

import threading
import timeit
import unittest


class TestControl(unittest.TestCase):

    def test_unused_object(self):
        """
        Create and ensure initial defaults are correct.
        """
        c = Control()
        self.assertEqual(c.finished, False)
        # Since we never started, the result is failed (False)
        self.assertEqual(c.wait_for_result(), False)

    def test_can_watch_state_changes(self):
        """
        Demonstrate a worker can move through each state.  Has
        some timing conditions which should be met on any system
        with reasonable performance.  This can thrash a CPU,
        but it's only for a few seconds.
        """
        start_time = timeit.default_timer()
        c = minimal_worker_example(sleep_ms=1000)
        self.assertEqual(c.finished, False)
        while "not started" == c.state:
            pass
        self.assertEqual(c.state, "setup")
        while "setup" == c.state:
            pass
        self.assertEqual(c.state, "working")
        while "working" == c.state:
            pass
        self.assertEqual(c.state, "teardown")
        while "teardown" == c.state:
            pass
        self.assertEqual(c.state, "complete")
        self.assertEqual(True, c.wait_for_result())
        elapsed = timeit.default_timer() - start_time
        self.assertGreaterEqual(elapsed, 3.0)
        self.assertLessEqual(elapsed, 4.0)

    def test_result_waits_until_ready(self):
        """
        Demonstrate calling result on a Control object
        will wait until the result is ready.
        """
        start_time = timeit.default_timer()
        c = minimal_worker_example()
        self.assertEqual(True, c.wait_for_result())
        elapsed = timeit.default_timer() - start_time
        self.assertGreaterEqual(elapsed, 0.3)

    def test_result_can_report_fail(self):
        """
        Demonstrate calling result on a Control object
        can return a failed value.
        """
        c = minimal_worker_example(report_success=False)
        self.assertEqual(False, c.wait_for_result())

    def test_result_can_timeout(self):
        """
        Demonstrate the timeout value can be exceeded
        while waiting on a worker.
        """
        c = minimal_worker_example(sleep_ms=1000)
        self.assertEqual(False, c.wait_for_result(timeout_in_seconds=1))
        self.assertEqual(True, c.wait_for_result(timeout_in_seconds=3))

    def test_destructor_waits(self):
        """
        Demonstrate a Control object waits at deletion
        for a result to be ready.
        """
        start_time = timeit.default_timer()
        minimal_worker_example()
        elapsed = timeit.default_timer() - start_time
        self.assertGreaterEqual(elapsed, 0.3)


if __name__ == '__main__':
    unittest.main()
