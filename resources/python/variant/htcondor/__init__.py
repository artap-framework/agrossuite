__all__ = ["Solver", "Process", "Job", "Connection", "ConnectionDictionary"]

from variant.htcondor.connection import Connection, ConnectionDictionary
from variant.htcondor.job import Solver, Process, Job

_cd = ConnectionDictionary()