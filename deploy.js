const FtpDeploy = require('ftp-deploy');
const ftpDeploy = new FtpDeploy();
const settings = require('./.ftpdeploy.js');

const config = {
    user: settings.user,
    // Password optional, prompted if none given
    password: settings.password,
    host: settings.host,
    port: settings.port,
    localRoot: __dirname + '/build',
    remoteRoot: settings.remoteRoot,
    include: ["tree3.bin",],
    // DON'T delete ALL existing files at destination before uploading, if true
    deleteRemote: settings.deleteRemote,
    // Passive mode is forced (EPSV command is not sent)
    forcePasv: true,
    // use sftp or ftp
    sftp: false,
};

ftpDeploy
    .deploy(config)
    .then((res) => {
        for (let i in res[0]) {
            console.log(res[0][i]);
        }
    })
    .catch((err) => console.log(err));
