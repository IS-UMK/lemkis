
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <unistd.h>

#include "CompanyPack.h"
#include "Printer.h"

struct shared_data {
  private:
    static constexpr int shm_value = 1;
    static constexpr mode_t shm_mode = 0666;
    static constexpr const char* shm_printers_name = "/printers_shm";
    static constexpr const char* shm_companies_name = "/companies_shm";

  public:
    int num_companies_;
    int num_printers_;

    printer* printers_;
    company_pack* companies_packs_;

    shared_data(int num_companies, int num_printers)
        : num_companies_(num_companies), num_printers_(num_printers) {
        init();
    }

    void init() {
        clean_up();
        init_shared_memory();
    }

    void init_shared_memory() {
        printers_ = init_printers_shm(num_printers_);
        companies_packs_ = init_companies_shm(num_companies_);
    }

    static auto init_printers_shm(int num_printers) -> printer* {
        int fd = shm_open(shm_printers_name, O_CREAT | O_RDWR, shm_mode);
        auto length = static_cast<long>(sizeof(printer) * num_printers);
        ftruncate(fd, length);
        return static_cast<printer*>(
            mmap(nullptr, length, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0));
    }

    static auto init_companies_shm(int num_companies) -> company_pack* {
        int fd = shm_open(shm_companies_name, O_CREAT | O_RDWR, shm_mode);
        auto length = static_cast<long>(sizeof(company_pack) * num_companies);
        ftruncate(fd, length);
        return static_cast<company_pack*>(
            mmap(nullptr, length, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0));
    }

    void clean_up() {
        clean_printers();
        clean_companies();
        printers_ = nullptr;
        companies_packs_ = nullptr;
        shm_unlink(shm_printers_name);
        shm_unlink(shm_companies_name);
    }

    void clean_printers() const {
        for (int i = 0; i < num_printers_; ++i) {
            auto& printer = printers_[i];
            sem_destroy(&printer.mutex);
        }
        munmap(printers_, sizeof(printer) * num_printers_);
    }

    void clean_companies() const {
        for (int i = 0; i < num_companies_; ++i) {
            auto& pack = companies_packs_[i];
            sem_destroy(&pack.notify_office_of_free_printer_condition);
            sem_destroy(&pack.notify_my_employees_from_office_condition);
            sem_destroy(&pack.notify_company_acquire_condition);
            sem_destroy(&pack.notify_company_release_condition);
        }
        munmap(companies_packs_, sizeof(company_pack) * num_companies_);
    }

    ~shared_data() { clean_up(); }
};