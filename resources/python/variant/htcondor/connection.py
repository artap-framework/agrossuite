from variant.htcondor.job import Process, Job

from os.path import split, splitext, basename, exists
from os import listdir, remove

import paramiko as pm
import tarfile as tar

class ConnectionDictionary:
    """General class for connections management."""

    def __init__(self):
        self._dictionary = dict()

    def __exit__(self, type, value, traceback):
        for connection in self._dictionary:
            connection.close()

    @property
    def dictionary(self):
        """Return dictionary of connections."""
        return self._dictionary

    @dictionary.setter
    def dictionary(self, value):
        raise RuntimeError('Object cannot be overwrite!')

    def connections(self):
        """Return list of connections."""
        return list(self._dictionary.values())

    def add_connection(self, connection, name=None):
        """Add new coonection.

        Args:
            connection: htcondor.Connection object.
            name: Connection name (default is None - use automatic name).
        """

        if not name:
            if hasattr(self, '_connection_index'): self._connection_index += 1
            else: self._connection_index = 0

            name = 'connection_{0:0{1}d}'.format(self._connection_index, 9)

        self._dictionary[name] = connection

    def exists(self, name):
        """Check if connection with name exists.

        Args:
            name: Connection name.
        """
        return name in self._dictionary.keys()

class Connection:
    """General class for connection."""

    def __init__(self, hostname, username, key_filename):
        """Initialization of new connection.

        Args:
            hostname: Address to cluster submiter.
            username: User name for connection.
            key_filename: Path to SSH public key.
        """

        self.hostname = hostname
        self.username = username
        self.key_filename = key_filename

        self.clean_remote_directory = True
        self.use_tape_archiver = True
        self.throw_job_files = True

        self._remote_directory = 'htcondor'
        self._local_directories = dict()
        self._remote_directories = dict()
        self._processes = dict()

        self._client = None
        self._sftp = None

    def __exit__(self, type, value, traceback):
        if self._sftp:
            self._sftp.close()

    def _connection_hash(self):
        return self.hostname + '_' + self.username + '_' + self.key_filename

    def sftp(self):
        """Return opened SFTP session connection."""
        if not self._sftp:
            self._sftp = self._client.open_sftp()
        return self._sftp

    def connect(self):
        """Create SFTP connection with cluster submiter."""
        from variant.htcondor import _cd

        if not _cd.exists(self._connection_hash()):
            client = pm.SSHClient()
            client.load_system_host_keys()
            client.set_missing_host_key_policy(pm.AutoAddPolicy())

            client.connect(hostname=self.hostname, username=self.username, key_filename=self.key_filename)
            _cd.add_connection(client, self._connection_hash())

        self._client = _cd.dictionary[self._connection_hash()]

    def jobs(self):
        """Return list of jobs."""
        return list(self._processes.keys())

    def processes(self, cluster):
        """Return list of processes for existing cluster.

        Args:
            cluster: Existing cluster (ClusterID).
        """
        return self._processes[cluster]

    def submit_job(self, job_directory):
        """Submit new job to cluster.

        Args:
            job_directory: Name or path to job directory.

        Returns:
                If job was corectly submited, return cluster (ClusterId)
                and list of processes (ProcId).
        """
        sftp = self.sftp()

        # check remote directory
        remote_directory_exist = False
        for file_name in sftp.listdir():
            if file_name != self._remote_directory:
                continue

            if (file_name == self._remote_directory and
                'd' in str(sftp.lstat(file_name)).split()[0]):
                remote_directory_exist = True
                break

        if not remote_directory_exist:
            sftp.mkdir(self._remote_directory)

        # job directory transfer
        remote_job_directory = '{0}/{1}'.format(self._remote_directory, split(basename(job_directory))[-1])
        sftp.mkdir(remote_job_directory)

        solvers = []
        for file_name in listdir(job_directory):
            sftp.put('{0}/{1}'.format(job_directory, file_name), '{0}/{1}'.format(remote_job_directory, file_name))
            if '.cmd' in file_name:
                solvers.append(file_name)

        # execute job
        if solvers:
            command = 'cd {0}; chmod +x *.cmd; condor_submit job.condor'.format(remote_job_directory)
        else:
            command = 'cd {0}; condor_submit job.condor'.format(remote_job_directory)
        stdin, stdout, stderr = self._client.exec_command(command)

        stdout_lines = stdout.readlines()
        stderr_lines = stderr.readlines()

        if len(stderr_lines) > 0:
            raise RuntimeError(stderr_lines)

        if len(stdout_lines) > 0:
            # TODO: predisposed for bugs
            if not 'submitted' in stdout_lines[1]:
                raise RuntimeError(stdout_lines)

            report = stdout_lines[1].replace(".\n", "").split(" job(s) submitted to cluster ")
            cluster = int(report[1])
            processes = list(range(int(report[0])))

            self._local_directories[cluster] = job_directory
            self._remote_directories[cluster] = remote_job_directory
            self._processes[cluster] = processes

            return cluster, processes

    def _remove_job_directory(self, cluster):
        command = 'rm -r {0}'.format(self._remote_directories[cluster])
        stdin, stdout, stderr = self._client.exec_command(command)

        stderr_lines = stderr.readlines()
        if len(stderr_lines) > 0:
            raise RuntimeError(stderr_lines)

    def remove_job(self, cluster, process=None):
        """Remove existing job.

        Args:
            cluster: Existing cluster (ClusterID).
            process: Specific job in the cluster (ProcId) (default is None - remove whole cluster).
        """

        command = 'condor_rm {0}'.format(cluster)
        if process:
            command += '.{1}'.format(process)
        stdin, stdout, stderr = self._client.exec_command(command)

        stdout_lines = stdout.readlines()
        stderr_lines = stderr.readlines()

        if len(stderr_lines) > 0:
            raise RuntimeError(stderr_lines)

        if len(stdout_lines) > 0:
            # TODO: predisposed for bugs
            if not 'marked for removal' in stdout_lines[0]:
                raise RuntimeError(stdout_lines)

            if self.clean_remote_directory:
                self._remove_job_directory(cluster)

            del self._local_directories[cluster]
            del self._remote_directories[cluster]
            del self._processes[cluster]

    def withdraw_job(self, cluster, job_directory=None):
        """Withdraw job from cluster.

        Args:
            cluster: Existing cluster (ClusterID).
            job_directory: Name or path to local job directory (default is None - use directory from submition).
        """
        sftp = self.sftp()

        remote_job_directory = self._remote_directories[cluster]
        if not job_directory:
            job_directory = self._local_directories[cluster]
            if not exists(job_directory):
                raise RuntimeError('Local job directory does not exists!')

        # get job files
        files_names = []
        for file_name in sftp.listdir(path=remote_job_directory):
            if file_name == '__pycache__':
                continue

            if self.throw_job_files:
                extension = splitext(file_name)[1]
                if extension in ['.condor_out', '.condor_log', '.condor_err', '.py']: continue

            files_names.append(file_name)

        if self.use_tape_archiver:
            # create job directory archive
            archive_file_name = '{0}.tar'.format(basename(remote_job_directory))
            command = 'cd {0}; tar -cvf {1} {2}'.format(remote_job_directory, archive_file_name, ' '.join(files_names))
            stdin, stdout, stderr = self._client.exec_command(command)
    
            stderr_lines = stderr.readlines()
            if len(stderr_lines) > 0:
                raise RuntimeError(stderr_lines)

            # get archive and extract
            local_archive_file_path = '{0}/{1}'.format(job_directory, archive_file_name)
            sftp.get('{0}/{1}'.format(remote_job_directory, archive_file_name), local_archive_file_path)
            sftp.remove('{0}/{1}'.format(remote_job_directory, archive_file_name))

            with tar.open(local_archive_file_path) as archive:
                for item in archive:
                    local_file_path = '{0}/{1}'.format(job_directory, item.name)
                    if exists(local_file_path):
                        remove(local_file_path)
                    archive.extract(item, path=job_directory)

            remove(local_archive_file_path)
        else:
            # get job directory file by file
            for file_name in files_names:
                local_file_path = '{0}/{1}'.format(job_directory, file_name)
                if exists(local_file_path):
                    remove(local_file_path)
                sftp.get('{0}/{1}'.format(remote_job_directory, file_name), local_file_path)

        # delete remote job directory
        if self.clean_remote_directory:
            self._remove_job_directory(cluster)

        del self._local_directories[cluster]
        del self._remote_directories[cluster]
        del self._processes[cluster]

    def job_info(self, cluster, attribute):
        """Get job information.

        Args:
            cluster: Existing cluster (ClusterID).
            attribute: Attribute or expression from the job ClassAd (e.g. JobStatus).

        Returns:
            If job is avaliable in queue or history, return value of attribute,
            otherwise return None.
        """

        # check queue
        command = 'condor_q {0} -format \"%v\" {1}'.format(cluster, attribute)
        stdin, stdout, stderr = self._client.exec_command(command)

        stdout_lines = stdout.readlines()
        stderr_lines = stderr.readlines()

        if len(stderr_lines) > 0:
            raise RuntimeError(stderr_lines)

        if len(stdout_lines) > 0:
            return list(stdout_lines[0])

        # check history
        command = 'condor_history {0} -format \"%v\" {1}'.format(cluster, attribute)
        stdin, stdout, stderr = self._client.exec_command(command)

        stdout_lines = stdout.readlines()
        stderr_lines = stderr.readlines()

        if len(stderr_lines) > 0:
            raise RuntimeError(stderr_lines)

        if len(stdout_lines) > 0:
            return list(stdout_lines[0])

        return None

    def job_completed(self, cluster, process=None):
        """Check if job was completed.
    
        Args:
            cluster: Existing cluster (ClusterID).
            process: Specific job in the cluster (ProcId) (default is None - check whole cluster).
        """

        cluster_status = self.job_info(cluster, 'JobStatus')
        if (len(cluster_status) != len(self.processes(cluster))):
            return False

        for process_status in cluster_status:
            if int(process_status) != 4:
                return False

        return True

if __name__ == "__main__":
    import pythonlab
    from time import sleep, time

    condor = Connection(hostname='tesla.fel.zcu.cz', username='fmach',
                        key_filename='/home/fmach/.ssh/id_rsa')
    condor.connect()
    condor.use_tape_archiver = True
    condor.throw_job_files = False
    #condor.clean_remote_directory = False

    job = Job('test')
    job.executable = "/bin/ls"
    for i in range(100):
        job.add_process(Process("/ -l"))

    job_directory = job.create(pythonlab.tempname())
    start = time()
    cluster, processes = condor.submit_job(job_directory)
    print('Submit: {0}'.format(time()-start))
    print('Cluster: {0}, processes: {1}'.format(cluster, processes))

    loops = 0
    start = time()
    while not condor.job_completed(cluster):
        sleep(1)
        loops += 1
    print('Solution: {0} ({1} loops)'.format(time()-start, loops))

    start = time()
    condor.withdraw_job(cluster)
    print('Withdraw: {0}'.format(time()-start))
    #print(listdir(job_directory))