from gild import Count
from gild import launch
from gild import State

import datetime
import threading
import timeit
import unittest


class TestCount(unittest.TestCase):

    def test_create_input(self):
        """
        Create and ensure initial defaults are correct.
        """
        input = Count()
        self.assertEqual(input.start, 1)
        self.assertEqual(input.end, 100)
        self.assertEqual(input.delay_ms, 1000)
        self.assertEqual(input.fail_after, State.INCOMPLETE)

    def test_modify_input(self):
        """
        Create and ensure initial defaults are correct.
        """
        input = Count()
        input.start = 1
        input.end = 10
        input.delay_ms = 100
        self.assertEqual(input.start, 1)
        self.assertEqual(input.end, 10)
        self.assertEqual(input.delay_ms, 100)
        return input

    def test_job_output_read_only(self):
        """
        Verify You can't change output in the job object.
        """
        input = self.test_modify_input()
        job = launch(input)
        with self.assertRaises(AttributeError):
            job.output.last = 5

    def test_can_run(self):
        """
        Verify we can count from 1 to 10.
        """
        input = self.test_modify_input()
        job = launch(input)
        while not job.finished:
            self.assertGreaterEqual(job.output.last, 0)
            self.assertLessEqual(job.output.last, 10)
        self.assertEqual( job.wait_for_result(), True)
        self.assertEqual(job.output.last, 10)

    def test_can_watch_state_changes(self):
        """
        Demonstrate a worker can move through each state.  Has
        some timing conditions which should be met on any system
        with reasonable performance.  This can thrash a CPU,
        but it's only for a few seconds.
        """
        input = self.test_modify_input()
        start_time = timeit.default_timer()
        job = launch(input)
        self.assertEqual(job.finished, False)
        self.assertEqual(job.state, State.SETUP)
        while State.SETUP == job.state:
            pass
        self.assertEqual(job.state, State.WORKING)
        while State.WORKING == job.state:
            pass
        self.assertEqual(job.state, State.TEARDOWN)
        while State.TEARDOWN == job.state:
            pass
        self.assertEqual(job.state, State.COMPLETE)
        self.assertEqual(True, job.wait_for_result())
        elapsed = timeit.default_timer() - start_time
        self.assertGreaterEqual(elapsed, 1.2)
        self.assertLessEqual(elapsed, 1.5)

    def test_result_waits_until_ready(self):
        """
        Demonstrate calling wait_for_result()
        will wait until the result is ready.
        """
        input = self.test_modify_input()
        start_time = timeit.default_timer()
        job = launch(input)
        self.assertEqual(True, job.wait_for_result())
        elapsed = timeit.default_timer() - start_time
        self.assertGreaterEqual(elapsed, 1.2)
        self.assertLessEqual(elapsed, 1.5)

    def test_elapsed_measures_only_working(self):
        """
        Demonstrate elapsed returns only the time spent
        in state.WORKING
        """
        input = Count()
        input.start = 1
        input.end = 2
        input.delay_ms = 500
        job = launch(input)
        self.assertEqual(True, job.wait_for_result())
        self.assertGreaterEqual(job.elapsed,
                                datetime.timedelta(seconds=1.0))
        self.assertLessEqual(job.elapsed,
                             datetime.timedelta(seconds=1.5))

    def test_result_can_fail_after_setup(self):
        """
        Demonstrate that failing in SETUP will
        run TEARDOWN but not WORKING.
        """
        input = self.test_modify_input()
        input.fail_after = State.SETUP
        start_time = timeit.default_timer()
        job = launch(input)
        self.assertEqual(False, job.wait_for_result())
        elapsed = timeit.default_timer() - start_time
        self.assertGreaterEqual(elapsed, 0.2)
        self.assertLessEqual(elapsed, 0.4)

    def test_result_can_fail_after_working(self):
        """
        Demonstrate that failing in WORKING will
        run TEARDOWN.
        """
        input = self.test_modify_input()
        input.fail_after = State.WORKING
        job = launch(input)
        while State.TEARDOWN != job.state:
            pass
        start_time = timeit.default_timer()
        while State.TEARDOWN == job.state:
            pass
        elapsed = timeit.default_timer() - start_time
        self.assertEqual(False, job.wait_for_result())
        self.assertGreaterEqual(elapsed, 0.1)
        self.assertLessEqual(elapsed, 0.2)

    def test_result_can_fail_after_teardown(self):
        """
        Demonstrate that failing in TEARDOWN will
        report failure.
        """
        input = self.test_modify_input()
        input.fail_after = State.TEARDOWN
        job = launch(input)
        self.assertEqual(False, job.wait_for_result())

    def test_result_can_timeout(self):
        """
        Demonstrate the timeout value can be exceeded
        while waiting on a worker.
        """
        input = self.test_modify_input()
        input.start = 1
        input.end = 2
        input.delay_ms = 600
        job = launch(input)
        self.assertEqual(False, job.wait_for_result(
                         timeout_in_seconds=1))
        self.assertEqual(True, job.wait_for_result(
                         timeout_in_seconds=2))

    def test_destructor_aborts(self):
        """
        Demonstrate a Job is aborted if it goes out of scope.
        """
        input = self.test_modify_input()
        start_time = timeit.default_timer()
        output = launch(input)
        elapsed = timeit.default_timer() - start_time
        self.assertLess(elapsed, 0.3)


if __name__ == '__main__':
    unittest.main()
