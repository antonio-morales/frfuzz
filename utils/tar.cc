/* SPDX-License-Identifier: AGPL-3.0-only */
#include "tar.h"

void tar_dir(std::string dir_path, std::string output_tar) {

    TAR *pTar;

    const char *tarFilename = output_tar.c_str();
    // char *srcDir = (char*)dir_path.c_str();

    // char *extractTo = "";
    tar_open(&pTar, tarFilename, NULL, O_WRONLY | O_CREAT, 0644, TAR_GNU);

    // tar_append_tree(pTar, srcDir, NULL);

    // For every file in the directory
    for (auto d : std::filesystem::directory_iterator(dir_path)) {

        // If it is a directory
        if (d.is_directory()) {

            tar_append_tree(pTar, (char *)d.path().string().c_str(), (char *)d.path().filename().string().c_str());

        } else {

            // Add the file to the current tar file
            tar_append_file(pTar, (char *)d.path().string().c_str(), (char *)d.path().filename().string().c_str());
        }
    }

    // tar_append_file(TAR *t, char *realname, char *savename);

    tar_append_eof(pTar);
    tar_close(pTar);
}

void untar(std::string input_tar, std::string output_path) {

    std::string command = "tar -xvf '" + input_tar + "' -C " + output_path;

    std::string output = run(command);

    // std::cout << output << std::endl;

    // char *argv[] = {"tar", "-xvf", const_cast<char *>(input_tar.c_str()), "-C", const_cast<char *>(output_path.c_str()), NULL};

    // execute(argv);

    // std::string command = "tar -xvf '" + input_tar + "' -C " + output_path;

    // int result = system(command.c_str());

    // std::cout << result << std::endl;
}

//__attribute__((no_sanitize("address")))
void untar__ERROR(std::string input_tar, std::string output_path) {

    TAR *pTar;

    if (tar_open(&pTar, input_tar.c_str(), NULL, O_RDONLY, 0644, TAR_GNU | TAR_NOOVERWRITE | TAR_VERBOSE) == -1) {
        std::cout << "Error in untar: " << strerror(errno) << std::endl;
        exit(-1);
    }

    int i = 0;
    while ((i = th_read(pTar)) == 0) {

        // th_print_long_ls(pTar);
        // th_print(pTar);

        std::cout << pTar->th_buf.name << std::endl;

        std::string realname = output_path + "/" + pTar->th_buf.name;

        tar_extract_file(pTar, (char *)realname.c_str());

        /*
        if(strcmp(pTar->th_buf.name,"ova.xml")==0)
        {
            puts(pTar->th_buf.name);
            tar_extract_regfile(pTar,pTar->th_buf.name);
        }


        if (TH_ISREG(pTar) && tar_skip_regfile(pTar) != 0)
        {
            fprintf(stderr, "tar_skip_regfile(): %s\n",
                strerror(errno));
            return;
        }

        */
    }

    tar_close(pTar);
}

void untar_backup(std::string input_tar, std::string output_path) {

    TAR *pTar;

    if (tar_open(&pTar, input_tar.c_str(), NULL, O_RDONLY, 0644, TAR_GNU | TAR_NOOVERWRITE | TAR_VERBOSE | TAR_IGNORE_CRC) == -1) {
        std::cout << "Error in untar: " << strerror(errno) << std::endl;
        exit(-1);
    }

    if (tar_extract_all(pTar, (char *)output_path.c_str()) != 0) {
        std::cout << "Error in untar: " << strerror(errno) << std::endl;
        exit(-1);
    }

    tar_close(pTar);
}