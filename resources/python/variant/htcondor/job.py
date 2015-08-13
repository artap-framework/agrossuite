from os.path import basename
from os import makedirs
from datetime import datetime
from shutil import copy

class Solver:
    """General class for inhouse solver."""

    def __init__(self, name, systems=['LINUX', 'WINDOWS'], architectures=None):
        """Initialization of solver.

            Args:
                name: Solver name.
                systems: List of supported operating systems (default is ['LINUX', 'WINDOWS']).
                architecture: List of supported computer architectures (default means that solver is independent on architecture).
        """

        self._name = name

        self._solvers = dict()
        for system in systems:
            if not architectures:
                self._solvers[system] = None
                self._architecture_independent = True
            else:
                self._solvers[system] = dict()
                self._architecture_independent = False
                for arch in architectures:
                    self._solvers[system][arch] = None

    def executable(self):
        """Return solver executable."""

        executable = '{0}.$$(OpSys)'.format(self._name)
        if not self._architecture_independent:
            executable += '.$$(Arch)'
        executable += '.cmd'

        return executable

    def add_solver_path(self, path, system, architecture=None):
        """Add path to solver for defined systems and architecture.

            Args:
                path: Path to existing solver (for example: /sbin/ifconfig or c:\windows\system32\ipconfig.exe).
                system: Operating systems.
                architecture: Computer architectures (default is None).
        """

        if not system in self._solvers.keys():
            raise KeyError('Operating system "{0} is not defined"!'.format(system))

        if (self._architecture_independent and architecture):
            raise RuntimeError('Architectures are not defined. Solver is architecture independent!')

        if self._architecture_independent:
            self._solvers[system] = path
        elif architecture:
            if not architecture in self._solvers[system].keys():
                raise KeyError('Architecture "{0} is not defined"!'.format(architecture))
            self._solvers[system][architecture] = path
        else:
            for arch in self._solvers[system].keys():
                self._solvers[system][arch] = path

    def script(self, system, architecture=None):
        script = ''
        if system == 'LINUX':
            script += '#!/bin/bash\n'

        if not architecture:
            script += self._solvers[system]
        else:
            script += self._solvers[system][architecture]

        if system == 'LINUX':
            script += ' $@'
        if system == 'WINDOWS':
            script += ' %*'

        return script

    def create(self, job_directory):
        """Create solver script in job directory."""

        if self._architecture_independent:
            for system in self._solvers.keys():
                file_name = '{0}/{1}.{2}.cmd'.format(job_directory, self._name, system)
                with open(file_name, 'w') as solver_file:
                    solver_file.write(self.script(system))
        else:
            for system, paths in self._solvers.items():
                for arch in paths.keys():
                    file_name = '{0}/{1}.{2}.{3}.cmd'.format(job_directory, self._name, system, arch)
                    with open(file_name, 'w') as solver_file:
                        solver_file.write(self.script(system, arch))

class Process:
    """General class for HTCondor process description."""

    def __init__(self, arguments, input_files=[]):
        """Initialization of process.

            Args:
                arguments: Process arguments for executable.
                input_files: List of process input files.
        """

        self.arguments = arguments
        self.input_files = input_files

class Job:
    """General class for HTCondor job description."""

    def __init__(self, name=None, universe='vanilla'):
        """Initialization of job.

            Args:
                name: Job name used as label.
                universe: HTCondor universe (default is vanilla).
        """

        self.name = name
        self.universe = universe

        self.executable = None
        self.solver = None

        self.requirements = None
        self.requests= {'request_cpus' : None,
                        'request_memory' : None,
                        'request_disk' : None,
                        'notification' : None,
                        'notify_user' : None}

        self._processes = list()

    def add_process(self, process):
        """Add new process to job.

            Args:
                process: Instance of Process class described new process.
        """

        if (process.__class__ == Process):
            self._processes.append(process)
        else:
            raise TypeError('Parameter process must be instance of Process class.')

    def description(self):
        """Returns content of submit description file."""

        # job description
        description = ''
        if self.name:
            description += '# {0}\n\n'.format(self.name)

        description += 'universe = {0}\n'.format(self.universe)

        if (self.solver and self.executable):
            raise RuntimeError('Executable and solver can not be defined together!')

        if self.executable:
            description += 'executable = {0}\n'.format(self.executable)
            description += 'transfer_executable = false\n\n'

        if self.solver:
            description += 'executable = {0}\n'.format(self.solver.executable())
            description += 'transfer_executable = true\n\n'

        description += 'log = $(cluster).condor_log\n'
        description += 'log_xml = True\n'

        description += 'output = $(cluster)_$(process).condor_out\n'
        description += 'error = $(cluster)_$(process).condor_err\n\n'

        description += 'should_transfer_files = IF_NEEDED\n'
        description += 'when_to_transfer_output = ON_EXIT\n'

        if self.requirements:
            description += "requirements = {0}\n\n".format(self.requirements)

        for key, value in self.requests.items():
            if value: description += "{0} = {1}\n".format(key, value)

        # processes description
        for process in self._processes:
            description += "\narguments = \"{0}\"\n".format(process.arguments)

            file_names = []
            for input_file in process.input_files:
                file_names.append(basename(input_file))

            if bool(len(process.input_files)):
                description += "transfer_input_files = {0}\n".format(",".join(file_names))
            description += "queue\n"

        return description

    def create(self, path):
        """Create job directory with unique name.

        Args:
            path: Path to jobs directory.

        Returns:
            If directory was created succesfully, returns path to directory.
        """

        job_directory = '{0}/{1}'.format(path, datetime.now().strftime("%Y-%m-%d-%H-%M-%S.%f"))
        makedirs(job_directory)
        
        with open('{0}/job.condor'.format(job_directory), 'w') as description_file:
            description_file.write(self.description())

        if self.solver:
            self.solver.create(job_directory)

        for process in self._processes:
            for file_name in process.input_files:
                copy(file_name, job_directory)

        return job_directory

if __name__ == "__main__":
    import pythonlab
    from os import listdir

    file_name = pythonlab.tempname('dat')
    with open(file_name, 'w') as file:
        file.write('test')

    job = Job('test')
    #job.executable = 'cmd'

    job.solver = Solver('test', architectures=['X86_64', 'INTEL'])
    job.solver.add_solver_path('/test', 'LINUX')
    job.solver.add_solver_path('c:\\programs_32\\test', 'WINDOWS', 'INTEL')
    job.solver.add_solver_path('c:\\programs\\test', 'WINDOWS', 'X86_64')

    job.requests['request_cpus'] = 1

    job.add_process(Process(arguments = '/ -a', input_files=[file_name]))
    job.add_process(Process(arguments = '/ -b'))

    print(job.description())
    path = job.create(pythonlab.tempdir())

    print(listdir(path))