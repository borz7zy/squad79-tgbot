#pragma once

#include "../main.hpp"

class Database : public Singleton<Database>
{
private:
    static constexpr const char *db_file = "./database.bin";
    std::unordered_map<std::string, std::streampos> index;
    std::fstream file;

    void loadIndex()
    {
        if (!file)
        {
            Logger::Get()->Log("Failed to open file!");
            rn;
        }

        index.clear();
        file.seekg(0, std::ios::beg);
        while (file.peek() != EOF)
        {
            std::streampos pos = file.tellg();
            uint32_t key_size, value_size;

            if (!file.read(reinterpret_cast<char *>(&key_size), sizeof(key_size)))
                break;

            std::vector<char> key_buf(key_size);
            if (!file.read(key_buf.data(), key_size))
                break;

            if (!file.read(reinterpret_cast<char *>(&value_size), sizeof(value_size)))
                break;

            file.seekg(value_size, std::ios::cur);

            index[std::string(key_buf.begin(), key_buf.end())] = pos;
        }

        Logger::Get()->Log("Index loaded!");
    }

    Database(const Database &) = delete;
    Database &operator=(const Database &) = delete;

public:
    Database()
    {
        Logger::Get()->Log("Database constructor called.");
        file.open(db_file, std::ios::binary | std::ios::in | std::ios::out | std::ios::app);
        loadIndex();
    }

    ~Database()
    {
        Logger::Get()->Log("Database destructor called.");
        if (file.is_open())
        {
            Logger::Get()->Log("Closing file in destructor.");
            file.close();
        }
        else
        {
            Logger::Get()->Log("File is not open in destructor.");
        }
    }

    template <typename N>
    void put(const std::string &key, const N &value)
    {
        if (!file.is_open())
        {
            Logger::Get()->Log("File is not open!");
            rn;
        }

        file.seekp(0, std::ios::end);
        std::streampos pos = file.tellp();
        uint32_t key_size = key.size();
        uint32_t value_size = sizeof(N);

        try
        {
            file.write(reinterpret_cast<const char *>(&key_size), sizeof(key_size));
            Logger::Get()->Log("Key size: %u", key_size);
            Logger::Get()->Log("File stream state: %d", file.rdstate());
            if (!file.good())
            {
                Logger::Get()->Log("Error writing key_size: %s", std::to_string(file.rdstate()).c_str());
                rn;
            }

            file.write(key.data(), key_size);
            if (!file.good())
            {
                Logger::Get()->Log("Error writing key: %s", std::to_string(file.rdstate()).c_str());
                rn;
            }

            file.write(reinterpret_cast<const char *>(&value_size), sizeof(value_size));
            if (!file.good())
            {
                Logger::Get()->Log("Error writing value_size: %s", std::to_string(file.rdstate()).c_str());
                rn;
            }

            file.write(reinterpret_cast<const char *>(&value), sizeof(N));
            if (!file.good())
            {
                Logger::Get()->Log("Error writing value: %s", std::to_string(file.rdstate()).c_str());
                rn;
            }
        }
        catch (const std::exception &e)
        {
            Logger::Get()->Log("Exception during write: %s", std::string(e.what()).c_str());
            file.clear(std::ios::badbit);
            rn;
        }
        catch (...)
        {
            Logger::Get()->Log("Unknown exception during write.");
            file.clear(std::ios::badbit);
            rn;
        }

        index[key] = pos;
        file.flush();

        Logger::Get()->Log("Data put to file!");
    }

    template <typename N>
    N get(const std::string &key)
    {
        if (index.find(key) == index.end() || !file.is_open())
        {
            Logger::Get()->Log("Key not found or file not open!");
            rn N{};
        }

        file.seekg(index[key]);

        uint32_t key_size;
        if (!file.read(reinterpret_cast<char *>(&key_size), sizeof(key_size)))
        {
            Logger::Get()->Log("Error reading key_size.");
            rn N{};
        }

        std::vector<char> key_buf(key_size);
        if (!file.read(key_buf.data(), key_size))
        {
            Logger::Get()->Log("Error reading key.");
            rn N{};
        }

        uint32_t value_size;
        if (!file.read(reinterpret_cast<char *>(&value_size), sizeof(value_size)))
        {
            Logger::Get()->Log("Error reading value_size.");
            rn N{};
        }

        N value;
        if (!file.read(reinterpret_cast<char *>(&value), sizeof(N)))
        {
            Logger::Get()->Log("Error reading value.");
            rn N{};
        }

        rn value;
    }

    template <typename N>
    std::vector<std::pair<std::string, N>> searchKeys(const std::string &pattern)
    {
        std::vector<std::pair<std::string, N>> results;
        std::regex regex_pattern(pattern);

        for (const auto &[key, pos] : index)
        {
            if (std::regex_match(key, regex_pattern))
            {
                file.seekg(pos);

                uint32_t key_size;
                if (!file.read(reinterpret_cast<char *>(&key_size), sizeof(key_size)))
                {
                    Logger::Get()->Log("Error reading key_size during search.");
                    continue;
                }

                std::vector<char> key_buf(key_size);
                if (!file.read(key_buf.data(), key_size))
                {
                    Logger::Get()->Log("Error reading key during search.");
                    continue;
                }

                uint32_t value_size;
                if (!file.read(reinterpret_cast<char *>(&value_size), sizeof(value_size)))
                {
                    Logger::Get()->Log("Error reading value_size during search.");
                    continue;
                }

                N value;
                if (!file.read(reinterpret_cast<char *>(&value), sizeof(N)))
                {
                    Logger::Get()->Log("Error reading value during search.");
                    continue;
                }

                results.emplace_back(key, value);
            }
        }

        rn results;
    }

    void remove(const std::string &key)
    {
        index.erase(key);
    }
};