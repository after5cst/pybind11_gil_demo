from gild import Count
from gild import launch

import threading
import timeit
import unittest


class TestCount(unittest.TestCase):

    def test_create_input(self):
        """
        Create and ensure initial defaults are correct.
        """
        input = Count()
        self.assertEqual(input.start, 0)
        self.assertEqual(input.end, 100)
        self.assertEqual(input.delay_ms, 1000)

    def test_modify_input(self):
        """
        Create and ensure initial defaults are correct.
        """
        input = Count()
        input.end = 10
        input.delay_ms = 200
        self.assertEqual(input.start, 0)
        self.assertEqual(input.end, 10)
        self.assertEqual(input.delay_ms, 200)
        return input

    def test_can_run(self):
        """
        Verify We can count to 10.
        """
        input = self.test_modify_input()
        job = launch(input)
        self.assertEqual( job.wait_for_result(), True)

    def atest_can_watch_state_changes(self):
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

    def atest_result_waits_until_ready(self):
        """
        Demonstrate calling result on a Control object
        will wait until the result is ready.
        """
        start_time = timeit.default_timer()
        c = minimal_worker_example()
        self.assertEqual(True, c.wait_for_result())
        elapsed = timeit.default_timer() - start_time
        self.assertGreaterEqual(elapsed, 0.3)

    def atest_result_can_report_fail(self):
        """
        Demonstrate calling result on a Control object
        can return a failed value.
        """
        c = minimal_worker_example(report_success=False)
        self.assertEqual(False, c.wait_for_result())

    def atest_result_can_timeout(self):
        """
        Demonstrate the timeout value can be exceeded
        while waiting on a worker.
        """
        c = minimal_worker_example(sleep_ms=1000)
        self.assertEqual(False, c.wait_for_result(timeout_in_seconds=1))
        self.assertEqual(True, c.wait_for_result(timeout_in_seconds=3))

    def atest_destructor_waits(self):
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
